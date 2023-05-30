//
// Created by cheng on 23-5-30.
//

#include "tcp_client.h"
#include "logger.h"
#include "order.pb.h"
#include "rpc_channel.h"
#include "tinypb_protocol.h"
#include "rpc_closure.h"
#include "config.h"


void test_tcp_client() {
    
    rpc::IPNetAddr::s_ptr addr = std::make_shared<rpc::IPNetAddr>("127.0.0.1", 8080);
    rpc::TcpClient client(addr);
    client.connect([addr, &client]() {
        LOG_DEBUG("conenct to [%s] success", addr->toString().c_str());
        std::shared_ptr<rpc::TinyPBProtocol> message = std::make_shared<rpc::TinyPBProtocol>();
        message->msg_id_ = "99998888";
        message->pb_data_ = "test pb data";
        
        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");
        
        if (!request.SerializeToString(&(message->pb_data_))) {
            LOG_ERROR("serilize error");
            return;
        }
        
        message->method_name_ = "Order.makeOrder";
        
        client.writeMessage(message, [request](rpc::AbstractProtocol::s_ptr msg_ptr) {
            LOG_DEBUG("send message success, request[%s]", request.ShortDebugString().c_str());
        });
        
        
        client.readMessage("99998888", [](rpc::AbstractProtocol::s_ptr msg_ptr) {
            std::shared_ptr<rpc::TinyPBProtocol> message = std::dynamic_pointer_cast<rpc::TinyPBProtocol>(msg_ptr);
            LOG_DEBUG("msg_id[%s], get response %s", message->msg_id_.c_str(), message->pb_data_.c_str());
            makeOrderResponse response;
            
            if(!response.ParseFromString(message->pb_data_)) {
                LOG_ERROR("deserialize error");
                return;
            }
            LOG_DEBUG("get response success, response[%s]", response.ShortDebugString().c_str());
        });
    });
}

void test_rpc_channel() {
    
    NEWRPCCHANNEL("127.0.0.1:8080", channel);
    
    // std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    
    NEWMESSAGE(makeOrderRequest, request);
    NEWMESSAGE(makeOrderResponse, response);
    
    request->set_price(100);
    request->set_goods("apple");
    
    NEWRPCCONTROLLER(controller);
    controller->SetMsgID("99998888");
    controller->SetTimeout(10000);
    
    std::shared_ptr<rpc::RpcClosure> closure = std::make_shared<rpc::RpcClosure>([request, response, channel, controller]() mutable {
        if (controller->GetErrCode() == 0) {
            LOG_INFO("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 执行业务逻辑
            if (response->order_id() == "xxx") {
                // xx
            }
        } else {
            LOG_ERROR("call rpc failed, request[%s], error code[%d], error info[%s]",
                     request->ShortDebugString().c_str(),
                     controller->GetErrCode(),
                     controller->GetErrorMsg().c_str());
        }
        
        LOG_INFO("now exit eventloop");
        // channel->getTcpClient()->stop();
        channel.reset();
    });
    
    CALLRPRC("127.0.0.1:8080", Order_Stub, makeOrder, controller, request, response, closure);
    
    // xxx
    // 协程
}

int main() {
    
    rpc::Config::SetGlobalConfig("config.yaml");
    
    rpc::Logger::InitGlobalLogger();
    
    test_tcp_client();
    //test_rpc_channel();
    
    auto loop = rpc::EventLoop::getEventLoopOfCurrentThread();
    loop->loop();
    
    LOG_INFO("test_rpc_channel end");
    
    return 0;
}