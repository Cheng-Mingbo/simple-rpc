//
// Created by cheng on 23-5-28.
//

#include "tcp_server.h"
#include "config.h"
#include "logger.h"
#include "tcp_connection.h"
#include "io_thread.h"
#include <utility>
#include <functional>


namespace rpc {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : local_addr_(std::move(local_addr)) {
    acceptor_ = std::make_shared<TcpAcceptor>(local_addr_);
    main_loop_ = EventLoop::getEventLoopOfCurrentThread();
    io_thread_group_ = new IOThreadGroup(Config::GetGlobalConfig().io_threads_);
    
    listen_channel_ = new Channel(acceptor_->getFd());
    listen_channel_->setNonBlock();
    listen_channel_->listen(Channel::TriggerEvent::kReadEvent, std::bind(&TcpServer::onAccept, this));
    main_loop_->addEpollEvent(listen_channel_);
    LOG_INFO("TcpServer listen on %s", local_addr_->toString().c_str());
}

void TcpServer::start() {
    io_thread_group_->start();
    main_loop_->loop();
}

void TcpServer::onAccept() {
    auto [client_fd, client_addr] = acceptor_->accept();
    NetAddr::s_ptr  peer_addr = client_addr;
    client_count_++;
    
    IOThread* io_thread = io_thread_group_->getNextIOThread();
    TcpConnection::s_ptr conn = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 256, peer_addr, local_addr_, TcpConnectionType::Server);
    conn->setState(TcpConnectionState::Connected);
    clients_.insert(conn);
    LOG_INFO("TcpServer::onAccept() new connection, fd = %d, peer_addr = %s, local_addr = %s", client_fd, peer_addr->toString().c_str(), local_addr_->toString().c_str());
}

}