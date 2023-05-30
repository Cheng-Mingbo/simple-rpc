//
// Created by cheng on 23-5-30.
//

#ifndef RPC_RPC_CHANNEL_H
#define RPC_RPC_CHANNEL_H
#include <google/protobuf/service.h>
#include "net_addr.h"
#include "timer_channel.h"
#include "tcp_client.h"
#include "rpc_controller.h"

namespace rpc {

#define NEWMESSAGE(type, var_name) \
  std::shared_ptr<type> var_name = std::make_shared<type>(); \

#define NEWRPCCONTROLLER(var_name) \
  std::shared_ptr<rpc::RpcController> var_name = std::make_shared<rpc::RpcController>(); \

#define NEWRPCCHANNEL(addr, var_name) \
  std::shared_ptr<rpc::RpcChannel> var_name = std::make_shared<rpc::RpcChannel>(std::make_shared<rpc::IPNetAddr>(addr)); \

#define CALLRPRC(addr, stub_name, method_name, controller, request, response, closure) \
  { \
  NEWRPCCHANNEL(addr, channel); \
  channel->init(controller, request, response, closure); \
  stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
  } \



class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel> {
  public:
    using s_ptr = std::shared_ptr<RpcChannel>;
    using controller_ptr = std::shared_ptr<google::protobuf::RpcController>;
    using message_ptr = std::shared_ptr<google::protobuf::Message>;
    using closure_ptr = std::shared_ptr<google::protobuf::Closure>;
    
    explicit RpcChannel(NetAddr::s_ptr peer_addr);
    ~RpcChannel() override = default;
    
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done) override;
    
    void init(controller_ptr controller, message_ptr request, message_ptr response, closure_ptr done);
    
    google::protobuf::RpcController* getController() const { return controller_.get(); }
    google::protobuf::Message* getRequest() const { return request_.get(); }
    google::protobuf::Message* getResponse() const { return response_.get(); }
    google::protobuf::Closure* getClosure() const { return done_.get(); }
    TcpClient* getClient() const { return client_.get(); }
    TimerChannel::s_ptr getTimer() const { return timer_; }
    
  private:
    NetAddr::s_ptr peer_addr_ {nullptr};
    NetAddr::s_ptr local_addr_ {nullptr};
    
    controller_ptr controller_ {nullptr};
    message_ptr request_ {nullptr};
    message_ptr response_ {nullptr};
    closure_ptr done_ {nullptr};
    
    bool is_init_ {false};
    TcpClient::s_ptr client_ {nullptr};
    TimerChannel::s_ptr timer_ {nullptr};
};


}
#endif //RPC_RPC_CHANNEL_H
