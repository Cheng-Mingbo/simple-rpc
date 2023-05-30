//
// Created by cheng on 23-5-30.
//
#include <mutex>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include "rpc_dispatcher.h"
#include "tcp_connection.h"
#include "error_code.h"
#include "logger.h"
#include "run_time.h"
#include "rpc_controller.h"
#include "rpc_closure.h"

namespace rpc {

RpcDispatcher::s_ptr RpcDispatcher::rpc_dispatcher_ = nullptr;
std::once_flag g_flag;


void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection *connection) {
    TinyPBProtocol::s_ptr req = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    TinyPBProtocol::s_ptr resp = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    
    std::string full_name = req->method_name_;
    std::string service_name;
    std::string method_name;
    if (!parseService(full_name, service_name, method_name)) {
        setTinyPBError(resp, ERROR_PARSE_SERVICE_NAME, "service name or method name is invalid");
        return;
    }
    
    auto iter = services_.find(service_name);
    if (iter == services_.end()) {
        setTinyPBError(resp, ERROR_SERVICE_NOT_FOUND, "service not found");
        LOG_ERROR("service not found: [%s]", service_name.c_str());
        return;
    }
    
    service_ptr service = iter->second;
    auto method = service->GetDescriptor()->FindMethodByName(method_name);
    
    if (method == nullptr) {
        setTinyPBError(resp, ERROR_METHOD_NOT_FOUND, "method not found");
        LOG_ERROR("method not found: [%s]", method_name.c_str());
        return;
    }
    
    google::protobuf::Message* request_msg = service->GetRequestPrototype(method).New();
    
    if (!request_msg->ParseFromString(req->pb_data_)) {
        setTinyPBError(resp, ERROR_FAILED_DESERIALIZE, "parse request failed");
        LOG_ERROR("parse request failed");
        if (request_msg != nullptr)
            delete request_msg;
        return;
    }
    
    LOG_INFO("%s | get rpc request[%s]", req->msg_id_.c_str(), request_msg->ShortDebugString().c_str());
    
    google::protobuf::Message* response_msg = service->GetResponsePrototype(method).New();
    
    RpcController rpcController;
    rpcController.SetLocalAddr(connection->getLocalAddr());
    rpcController.SetPeerAddr(connection->getPeerAddr());
    rpcController.SetMsgID(req->msg_id_);
    
    RunTime::GetRunTime()->msg_id_ = req->msg_id_;
    RunTime::GetRunTime()->method_name_ = method_name;
    service->CallMethod(method, &rpcController, request_msg, response_msg, nullptr);
    
    if (!response_msg->SerializeToString(&(resp->pb_data_))) {
        setTinyPBError(resp, ERROR_FAILED_SERIALIZE, "serialize response failed");
        LOG_ERROR("serialize response failed");
        if (request_msg != nullptr)
            delete request_msg;
        if (response_msg != nullptr)
            delete response_msg;
        return;
    }
    
    resp->err_code_ = 0;
    LOG_INFO("%s | get rpc response[%s]", req->msg_id_.c_str(), response_msg->ShortDebugString().c_str());
    
    if (request_msg != nullptr) {
        delete request_msg;
        request_msg = nullptr;
    }
    if (response_msg != nullptr) {
        delete response_msg;
        response_msg = nullptr;
    }
}

RpcDispatcher::s_ptr RpcDispatcher::GetRpcDispatcher() {
    if (rpc_dispatcher_ == nullptr) {
        std::call_once(g_flag, []() {
            rpc_dispatcher_.reset(new RpcDispatcher());
        });
    }
    return rpc_dispatcher_;
}

void RpcDispatcher::registerService(RpcDispatcher::service_ptr service) {
    std::string  service_name = service->GetDescriptor()->full_name();
    services_[service_name] = service;
}

void RpcDispatcher::setTinyPBError(TinyPBProtocol::s_ptr msg, int32_t err_code, const std::string &err_msg) {
    msg->err_code_ = err_code;
    msg->err_msg_ = err_msg;
    msg->err_msg_len_ = err_msg.length();
}

bool RpcDispatcher::parseService(const std::string &full_name, std::string &service, std::string &method) {
    if (full_name.empty()) {
        LOG_ERROR("service name is empty");
        return false;
    }
    size_t i = full_name.find_first_of('.');
    if (i == std::string::npos) {
        LOG_ERROR("service name is invalid: [%s]", full_name.c_str());
        return false;
    }
    service = full_name.substr(0, i);
    method = full_name.substr(i + 1, full_name.length() - i - 1);
    LOG_DEBUG("service name: [%s], method name: [%s]", service.c_str(), method.c_str());
    return true;
}
}