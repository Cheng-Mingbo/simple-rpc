//
// Created by cheng on 23-5-30.
//

#ifndef RPC_RPC_CLOSURE_H
#define RPC_RPC_CLOSURE_H
#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
namespace rpc {
  class RpcClosure : public google::protobuf::Closure {
      public:
      using s_ptr = std::shared_ptr<RpcClosure>;
      using Callback = std::function<void()>;
      RpcClosure(Callback callback) : callback_(std::move(callback)) {}
      void Run() override {
        if (callback_) {
          callback_();
        }
      }
    private:
      Callback callback_;
  };
}

#endif //RPC_RPC_CLOSURE_H
