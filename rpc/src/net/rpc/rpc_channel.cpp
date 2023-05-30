//
// Created by cheng on 23-5-30.
//

#include "rpc_channel.h"
#include "tinypb_protocol.h"
#include "rpc_controller.h"
#include "msg_id_util.h"
#include "logger.h"
#include "error_code.h"
#include <utility>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace rpc {
RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : peer_addr_(std::move(peer_addr)) {
    client_ = std::make_shared<TcpClient>(peer_addr_);
}

void
RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method, google::protobuf::RpcController *controller,
                       const google::protobuf::Message *request, google::protobuf::Message *response,
                       google::protobuf::Closure *done) {
    auto req_protocol = std::make_shared<rpc::TinyPBProtocol>();
    auto my_controller = dynamic_cast<rpc::RpcController *>(controller);
    if (my_controller == nullptr) {
        LOG_ERROR("RpcChannel::CallMethod() error, controller is nullptr");
        return;
    }
    
    if (my_controller->GetMsgID().empty()) {
        req_protocol->msg_id_ = MsgIDUtil::GetMsgID();
        my_controller->SetMsgID(req_protocol->msg_id_);
    } else {
        req_protocol->msg_id_ = my_controller->GetMsgID();
    }
    
    req_protocol->method_name_ = method->full_name();
    LOG_INFO("RpcChannel::CallMethod() method_name: {%s}", req_protocol->method_name_.c_str());
    
    if (!is_init_) {
        std::string err_info = "RpcChannel::CallMethod() error, channel is not init";
        my_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
        LOG_ERROR("%s", err_info.c_str());
        return;
    }
    
    if (!request->SerializeToString(&(req_protocol->pb_data_))) {
        std::string err_info = "RpcChannel::CallMethod() error, request SerializeToString failed";
        my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
        LOG_ERROR("%s", err_info.c_str());
        return;
    }
    
    s_ptr channel = shared_from_this();
    timer_ = std::make_shared<TimerChannel>(my_controller->GetTimeout(), false, [my_controller, channel]() mutable  {
        my_controller->StartCancel();
        my_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "RpcChannel::CallMethod() error, call timeout");
        
        if (channel->getClosure()) {
            channel->getClosure()->Run();
        }
        channel.reset();
    });
    
    client_->connect([req_protocol, channel]() mutable {
        RpcController *my_controller = dynamic_cast<RpcController *>(channel->getController());
        
        if (channel->getClient()->getConnectErrorCode() != 0) {
            my_controller->SetError(channel->getClient()->getConnectErrorCode(),
                                    "RpcChannel::CallMethod() error, connect failed");
            LOG_ERROR("RpcChannel::CallMethod() error, connect failed");
            return;
        }
        
        LOG_INFO("RpcChannel::CallMethod() connect success");
        
        channel->getClient()->writeMessage(req_protocol, [req_protocol, channel, my_controller](
                const AbstractProtocol::s_ptr &) mutable {
            LOG_INFO("%s | send rpc request success.", req_protocol->msg_id_.c_str());
            
            channel->getClient()->readMessage(req_protocol->msg_id_, [channel, my_controller](AbstractProtocol::s_ptr msg) mutable {
                auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(msg);
                LOG_INFO("%s | recv rpc response success.", rsp_protocol->msg_id_.c_str());
                
                channel->getTimer()->setCancel(true);
                
                if (!(channel->getResponse()->ParseFromString(rsp_protocol->pb_data_))) {
                    LOG_ERROR("%s | recv rpc response failed, parse failed.", rsp_protocol->msg_id_.c_str());
                    my_controller->SetError(rsp_protocol->err_code_, rsp_protocol->err_msg_);
                    return;
                }
                
                if (rsp_protocol->err_code_ != 0) {
                    LOG_ERROR("%s | recv rpc response failed, err_code: %d, err_msg: %s",
                              rsp_protocol->msg_id_.c_str(), rsp_protocol->err_code_, rsp_protocol->err_msg_.c_str());
                    my_controller->SetError(rsp_protocol->err_code_, rsp_protocol->err_msg_);
                    return;
                }
                
                LOG_INFO("call rpc success, msg_id: %s", rsp_protocol->msg_id_.c_str());
                
                if (!my_controller->IsCanceled() && channel->getClosure()) {
                    channel->getClosure()->Run();
                }
                channel.reset();
            });
        });
        
    });
}

void RpcChannel::init(RpcChannel::controller_ptr controller, RpcChannel::message_ptr request,
                      RpcChannel::message_ptr response, RpcChannel::closure_ptr done) {
    if (is_init_) {
        return;
    }
    
    controller_ = std::move(controller);
    request_ = std::move(request);
    response_ = std::move(response);
    done_ = std::move(done);
    is_init_ = true;
}
}