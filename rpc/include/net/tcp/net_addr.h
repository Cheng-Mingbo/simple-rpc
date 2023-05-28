//
// Created by cheng on 23-5-28.
//

#ifndef RPC_NET_ADDR_H
#define RPC_NET_ADDR_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rpc {
class NetAddr {
  public:
    using s_ptr = std::shared_ptr<NetAddr>;
    virtual ~NetAddr() = default;
    virtual sockaddr* getSockAddr() const = 0;
    virtual socklen_t getAddrLen() const = 0;
    virtual int getFamily() = 0;
    virtual std::string toString() = 0;
    virtual bool checkValid() = 0;
};

class IPNetAddr : public NetAddr {
  public:
    
    IPNetAddr(std::string  ip, uint16_t port);
    IPNetAddr(const std::string& addr);
    IPNetAddr(const sockaddr_in& addr);
    
    sockaddr* getSockAddr() const override;
    socklen_t getAddrLen() const override;
    int getFamily() override;
    std::string toString() override;
    bool checkValid() override;
    
    
  private:
    std::string ip_;
    uint16_t port_;
    sockaddr_in addr_{};
};


}
#endif //RPC_NET_ADDR_H
