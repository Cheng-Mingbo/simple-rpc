//
// Created by cheng on 23-5-25.
//

#ifndef RPC_APPENDER_H
#define RPC_APPENDER_H
#include <vector>
#include <string>
#include "logevent.h"

namespace rpc {
class Appender {
  public:
    Appender() = default;
    
    virtual ~Appender() = default;
    
    virtual void append(const std::string &) = 0;
    
    virtual void append_app(const std::string &) = 0;
    
    virtual void swap(std::vector<std::vector<std::string>> &) = 0;
};
}
#endif //RPC_APPENDER_H
