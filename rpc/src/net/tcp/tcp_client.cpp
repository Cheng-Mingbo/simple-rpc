//
// Created by cheng on 23-5-29.
//

#include "tcp_client.h"
#include "logger.h"
#include "channel_group.h"
#include "error_code.h"

#include <utility>
#include <cstring>

namespace rpc {

TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : peer_addr_(std::move(peer_addr)) {
    loop_ = EventLoop::getEventLoopOfCurrentThread();
    fd_ = ::socket(peer_addr_->getFamily(), SOCK_STREAM, 0);
    if (fd_ < 0) {
        LOG_ERROR("create socket error: %s", strerror(errno));
        exit(1);
    }
    channel_ = ChannelGroup::GetChannelGroup()->getChannel(fd_);
    channel_->setNonBlock();
    
    connection_ = std::make_shared<TcpConnection>(loop_, fd_, 128, peer_addr_, nullptr, TcpConnectionType::Client);
    connection_->setConnectionType(TcpConnectionType::Client);
}

TcpClient::~TcpClient() {
    LOG_DEBUG("TcpClient::~TcpClient() fd: %d", fd_);
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

void TcpClient::connect(std::function<void()> done) {
    int ret = ::connect(fd_, peer_addr_->getSockAddr(), peer_addr_->getAddrLen());
    if (ret == 0) {
        LOG_DEBUG("connect [%s] success", peer_addr_->toString().c_str());
        connection_->setState(TcpConnectionState::Connected);
        initLocalAddr();
        if (done) {
            done();
        }
    } else if (ret == -1) {
        if (errno == EINPROGRESS) {
            channel_->listen(Channel::TriggerEvent::kWriteEvent, [this, done]() {
               int ret = ::connect(fd_, peer_addr_->getSockAddr(), peer_addr_->getAddrLen());
               if ((ret < 0 && errno == EISCONN) || (ret == 0)) {
                     LOG_DEBUG("connect [%s] success", peer_addr_->toString().c_str());
                     connection_->setState(TcpConnectionState::Connected);
                     initLocalAddr();
                } else {
                   if (errno == ECONNREFUSED) {
                       connect_error_code_ = ERROR_PEER_CLOSED;
                       connect_error_msg_ = "connect refused, sys error: " + std::string(strerror(errno));
                   } else {
                       connect_error_code_ = ERROR_FAILED_CONNECT;
                       connect_error_msg_ = "connect failed, sys error: " + std::string(strerror(errno));
                   }
                   LOG_ERROR("connect [%s] failed, sys error: %s", peer_addr_->toString().c_str(), strerror(errno));
                   ::close(fd_);
                   fd_ = ::socket(peer_addr_->getFamily(), SOCK_STREAM, 0);
               }
               loop_->removeEpollEvent(channel_);
               LOG_DEBUG("now begain to call done");
                if (done) {
                    done();
                }
            });
            loop_->addEpollEvent(channel_);
            if (!loop_->isLooping()) {
                loop_->loop();
            }
        } else {
            LOG_ERROR("connect [%s] failed, sys error: %s", peer_addr_->toString().c_str(), strerror(errno));
            connect_error_code_ = ERROR_FAILED_CONNECT;
            connect_error_msg_ = "connect failed, sys error: " + std::string(strerror(errno));
            if (done) {
                done();
            }
        }
    }
}

void TcpClient::stop() {
    if (loop_->isLooping()) {
        loop_->stop();
    }
}

void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    connection_->pushSendMessage(message, done);
    connection_->listenWrite();
}

void TcpClient::readMessage(const std::string &msg_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    connection_->pushReadMessage(msg_id, done);
    connection_->listenRead();
}

void TcpClient::initLocalAddr() {
    sockaddr_in local_addr{};
    socklen_t addr_len = sizeof(local_addr);
    
    int ret = ::getsockname(fd_, (sockaddr *) &local_addr, &addr_len);
    if (ret != 0) {
        LOG_ERROR("getsockname error: %s", strerror(errno));
        return;
    }
    local_addr_ = std::make_shared<IPNetAddr>(local_addr);
}

void TcpClient::addTimerEvent(TimerChannel::s_ptr timer) {
    loop_->addTimer(timer);
}

}