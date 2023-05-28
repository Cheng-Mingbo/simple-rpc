//
// Created by cheng on 23-5-28.
//

#ifndef RPC_ACCEPTOR_H
#define RPC_ACCEPTOR_H
#include "net_addr.h"

namespace rpc {
class TcpAcceptor {
  public:
    using s_ptr = std::shared_ptr<TcpAcceptor>;
    explicit TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor() = default;
    
    std::pair<int, NetAddr::s_ptr> accept();
    int getFd() const { return listen_fd_; }
  
  private:
    NetAddr::s_ptr local_addr_;
    int family_ {-1};
    int listen_fd_{};
};
}
#endif //RPC_ACCEPTOR_H
