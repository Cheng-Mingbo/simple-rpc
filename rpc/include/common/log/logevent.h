//
// Created by cheng on 23-5-25.
//

#ifndef RPC_LOGEVENT_H
#define RPC_LOGEVENT_H
#include "loglevel.h"
namespace rpc {

class LogEvent {
  public:
    explicit LogEvent(LogLevel level) : level_(level) {}
    
    [[nodiscard]] std::string getFileName() const { return file_name_; }
    [[nodiscard]] LogLevel getLevel() const { return level_; }
    std::string toString();
  
  private:
    LogLevel level_;
    std::string file_name_;
    int32_t line_{};
    int32_t pid_{};
    int32_t tid_{};
};

}
#endif //RPC_LOGEVENT_H
