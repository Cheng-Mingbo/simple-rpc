//
// Created by cheng on 23-5-30.
//

#ifndef RPC_RPC_CONTROLLER_H
#define RPC_RPC_CONTROLLER_H
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <string>

#include "net_addr.h"

namespace rpc {
class RpcController : public google::protobuf::RpcController {
  public:
    // Resets the RpcController to its initial state so that it may be reused in
    // a new call.  Must not be called while an RPC is in progress.
    void Reset() override;
    
    // After a call has finished, returns true if the call failed.  The possible
    // reasons for failure depend on the RPC implementation.  Failed() must not
    // be called before a call has finished.  If Failed() returns true, the
    // contents of the response message are undefined.
    bool Failed() const override;
    
    // If Failed() is true, returns a human-readable description of the error.
    std::string ErrorText() const override;
    
    // Advises the RPC system that the caller desires that the RPC call be
    // canceled.  The RPC system may cancel it immediately, may wait awhile and
    // then cancel it, or may not even cancel the call at all.  If the call is
    // canceled, the "done" callback will still be called and the RpcController
    // will indicate that the call failed at that time.
    void StartCancel() override;
    
    // Server-side methods ---------------------------------------------
    // These calls may be made from the server side only.  Their results
    // are undefined on the client side (may crash).
    
    // Causes Failed() to return true on the client side.  "reason" will be
    // incorporated into the message returned by ErrorText().  If you find
    // you need to return machine-readable information about failures, you
    // should incorporate it into your response protocol buffer and should
    // NOT call SetFailed().
    void SetFailed(const std::string& reason) override;
    
    // If true, indicates that the client canceled the RPC, so the server may
    // as well give up on replying to it.  The server should still call the
    // final "done" callback.
    bool IsCanceled() const override;
    
    // Asks that the given callback be called when the RPC is canceled.  The
    // callback will always be called exactly once.  If the RPC completes without
    // being canceled, the callback will be called after completion.  If the RPC
    // has already been canceled when NotifyOnCancel() is called, the callback
    // will be called immediately.
    //
    // NotifyOnCancel() must be called no more than once per request.
    void NotifyOnCancel(google::protobuf::Closure* callback) override;
    
    
    void SetError(int32_t err_code, const std::string& error_msg) {
      err_code_ = err_code;
      error_msg_ = error_msg;
    }
    
    int32_t GetErrCode() const {
      return err_code_;
    }
    
    const std::string& GetErrorMsg() const {
      return error_msg_;
    }
    
    void SetMsgID(const std::string& msg_id) {
      msg_id_ = msg_id;
    }
    
    const std::string& GetMsgID() const {
      return msg_id_;
    }
    
    void SetLocalAddr(const NetAddr::s_ptr& addr) {
      local_addr_ = addr;
    }
    
    const NetAddr::s_ptr& GetLocalAddr() const {
      return local_addr_;
    }
    
    void SetPeerAddr(const NetAddr::s_ptr& addr) {
      peer_addr_ = addr;
    }
    
    const NetAddr::s_ptr& GetPeerAddr() const {
      return peer_addr_;
    }
    
    void SetTimeout(int timeout_ms) {
      timeout_ms_ = timeout_ms;
    }
    
    int GetTimeout() const {
      return timeout_ms_;
    }
    
  private:
    int32_t err_code_ {0};
    std::string error_msg_;
    std::string msg_id_;
    
    bool is_failed_ {false};
    bool is_canceled_ {false};
    
    NetAddr::s_ptr  local_addr_;
    NetAddr::s_ptr  peer_addr_;
    
    int timeout_ms_ {1000};
};
}
#endif //RPC_RPC_CONTROLLER_H
