//
// Created by cheng on 23-5-30.
//
#include <google/protobuf/service.h>
#include "order.pb.h"
#include "net_addr.h"
#include "tcp_server.h"
#include "logger.h"
#include "config.h"
#include "rpc_dispatcher.h"

class OrderImpl : public Order {
  public:
    void makeOrder(google::protobuf::RpcController* controller,
                   const ::makeOrderRequest* request,
                   ::makeOrderResponse* response,
                   ::google::protobuf::Closure* done) {
        LOG_DEBUG("start sleep 5s");
        sleep(5);
        LOG_DEBUG("end sleep 5s");
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20230514");
        LOG_DEBUG("call makeOrder success");
    }
    
};


int main() {
    rpc::Config::SetGlobalConfig("config.yaml");
    rpc::Logger::InitGlobalLogger();
    
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    rpc::RpcDispatcher::GetRpcDispatcher()->registerService(service);
    
    rpc::IPNetAddr::s_ptr addr = std::make_shared<rpc::IPNetAddr>("127.0.0.1", rpc::Config::GetGlobalConfig().port_);
    
    rpc::TcpServer tcp_server(addr);
    
    tcp_server.start();
    
    return 0;
}