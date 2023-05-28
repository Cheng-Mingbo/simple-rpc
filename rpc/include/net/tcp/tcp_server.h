//
// Created by cheng on 23-5-28.
//

#ifndef RPC_TCP_SERVER_H
#define RPC_TCP_SERVER_H
#include <set>
#include "acceptor.h"
#include "net_addr.h"
#include "eventloop.h"
#include "io_thread_group.h"
#include "tcp_connection.h"
#include "channel.h"


namespace rpc {

class TcpServer {
  public:
    explicit TcpServer(NetAddr::s_ptr local_addr);
    ~TcpServer() = default;
    
    void start();
    void onAccept();
  private:
    TcpAcceptor::s_ptr acceptor_;
    NetAddr::s_ptr local_addr_;
    
    EventLoop* main_loop_ {nullptr};
    IOThreadGroup* io_thread_group_ {nullptr};
    Channel* listen_channel_ {nullptr};
    
    int client_count_ {0};
    std::set<TcpConnection::s_ptr> clients_;
};



}

#endif //RPC_TCP_SERVER_H
