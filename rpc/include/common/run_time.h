//
// Created by cheng on 23-5-30.
//

#ifndef RPC_RUN_TIME_H
#define RPC_RUN_TIME_H
#include <memory>
namespace rpc {
class RunTime {
  public:
    using s_ptr = std::shared_ptr<RunTime>;
    RunTime(const RunTime&) = delete;
    RunTime& operator=(const RunTime&) = delete;
    
  public:
    static s_ptr GetRunTime();
    std::string msg_id_;
    std::string method_name_;
  private:
    RunTime() = default;
    
};
}
#endif //RPC_RUN_TIME_H
