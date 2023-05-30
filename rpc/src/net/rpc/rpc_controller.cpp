//
// Created by cheng on 23-5-30.
//

#include "rpc_controller.h"


namespace rpc {
void RpcController::Reset() {
    err_code_ = 0;
    error_msg_.clear();
    msg_id_.clear();
    is_failed_ = false;
    is_canceled_ = false;
    local_addr_.reset();
    peer_addr_.reset();
    timeout_ms_ = 1000;
}

bool RpcController::Failed() const {
    return is_failed_;
}

std::string RpcController::ErrorText() const {
    return error_msg_;
}

void RpcController::StartCancel() {
    is_canceled_ = true;
}

bool RpcController::IsCanceled() const {
    return is_canceled_;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {
}

void RpcController::SetFailed(const std::string &reason) {
    error_msg_ = reason;
}

}