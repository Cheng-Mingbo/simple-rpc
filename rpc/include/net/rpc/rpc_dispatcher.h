//
// Created by cheng on 23-5-30.
//

#ifndef RPC_RPC_DISPATCHER_H
#define RPC_RPC_DISPATCHER_H
#include <map>
#include <memory>
#include <google/protobuf/service.h>
#include "abstract_protocol.h"
#include "tinypb_protocol.h"

namespace rpc {
class TcpConnection;
class RpcDispatcher {
  public:
    using s_ptr = std::shared_ptr<RpcDispatcher>;
    using service_ptr = std::shared_ptr<google::protobuf::Service>;
    RpcDispatcher(const RpcDispatcher&) = delete;
    RpcDispatcher& operator=(const RpcDispatcher&) = delete;
    
    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection);
    void registerService(service_ptr service);
    
    void setTinyPBError(TinyPBProtocol::s_ptr msg, int32_t err_code, const std::string& err_msg);
    
  public:
    static s_ptr GetRpcDispatcher();
    
    
  private:
    RpcDispatcher() = default;
    
    bool parseService(const std::string& full_name, std::string& service, std::string& method);
    
  private:
    static s_ptr rpc_dispatcher_;
    std::map<std::string, std::shared_ptr<google::protobuf::Service>> services_;
};
}
#endif //RPC_RPC_DISPATCHER_H
