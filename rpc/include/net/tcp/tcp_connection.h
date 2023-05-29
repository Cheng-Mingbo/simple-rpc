//
// Created by cheng on 23-5-28.
//

#ifndef RPC_TCPCONNECTION_H
#define RPC_TCPCONNECTION_H
#include "net_addr.h"
#include "buffer.h"
#include "eventloop.h"
#include "channel.h"
#include "abstract_coder.h"
#include "abstract_protocol.h"

namespace rpc {
enum class TcpConnectionState {
    NotConnected,
    Connected,
    HalfClosing,
    Closed
};

enum class TcpConnectionType {
    Client,
    Server
};


class TcpConnection {
  public:
    using s_ptr = std::shared_ptr<TcpConnection>;
    explicit TcpConnection(EventLoop* loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType type = TcpConnectionType::Server);
    ~TcpConnection();
    
    
    void onRead();
    void excute();
    void onWrite();
    
    void setState(TcpConnectionState state) { state_ = state; }
    TcpConnectionState getState() const { return state_; }
    
    void clear();
    
    void shutdown();
    
    void setConnectionType(TcpConnectionType type) { type_ = type; }
    
    void listenWrite();
    
    void listenRead();
    
    void pushSendMessage(AbstractProtocol::s_ptr protocol, std::function<void(AbstractProtocol::s_ptr)> done);
    void pushReadMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);
    
    NetAddr::s_ptr getPeerAddr() const { return peer_addr_; }
    NetAddr::s_ptr getLocalAddr() const { return local_addr_; }
  
  
  private:
    EventLoop* loop_ {nullptr};
    Channel* channel_ {nullptr};
    AbstractCoder* coder_ {nullptr};
    TcpConnectionState state_ {TcpConnectionState::NotConnected};
    
    NetAddr::s_ptr local_addr_;
    NetAddr::s_ptr peer_addr_;
    
    TcpBuffer::s_ptr input_buffer_;
    TcpBuffer::s_ptr output_buffer_;
    
    TcpConnectionType type_ {TcpConnectionType::Client};
    int fd_ {-1};
    
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> write_dons_;
    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> read_dons_;
};
}
#endif //RPC_TCPCONNECTION_H
