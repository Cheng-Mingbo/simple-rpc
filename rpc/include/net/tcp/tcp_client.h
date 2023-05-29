//
// Created by cheng on 23-5-29.
//

#ifndef RPC_TCP_CLIENT_H
#define RPC_TCP_CLIENT_H
#include <memory>
#include "net_addr.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "abstract_protocol.h"
#include "timer_channel.h"

namespace rpc {
class TcpClient {
  public:
    using s_ptr = std::shared_ptr<TcpClient>;
    
    explicit TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();
    
    void connect(std::function<void()> done);
    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);
    void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);
    
    void stop();
    
    int getConnectErrorCode() const { return connect_error_code_; }
    std::string getConnectErrorMsg() const { return connect_error_msg_; }
    
    NetAddr::s_ptr getPeerAddr() const { return peer_addr_; }
    NetAddr::s_ptr getLocalAddr() const { return local_addr_; }
    
    void initLocalAddr();
    void addTimerEvent(TimerChannel::s_ptr timer);
    
  private:
    NetAddr::s_ptr peer_addr_;
    NetAddr::s_ptr local_addr_;
    
    EventLoop* loop_;
    int fd_ {-1};
    Channel* channel_{};
    
    TcpConnection::s_ptr connection_;
    int connect_error_code_ {0};
    std::string connect_error_msg_;
};
}
#endif //RPC_TCP_CLIENT_H
