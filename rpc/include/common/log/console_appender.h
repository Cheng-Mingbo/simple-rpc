//
// Created by cheng on 23-5-25.
//

#ifndef RPC_CONSOLE_APPENDER_H
#define RPC_CONSOLE_APPENDER_H
#include <iostream>
#include "appender.h"

namespace rpc {
class ConsoleAppender : public Appender {
  public:
    explicit ConsoleAppender(std::ostream& os = std::cout) : os_(os) {};
    ~ConsoleAppender() override = default;
    
    void append(const std::string& event) override;
    void append_app(const std::string& event) override;
    void swap(std::vector<std::vector<std::string>>&) override;
    
  private:
    std::ostream& os_;
};
}
#endif //RPC_CONSOLE_APPENDER_H
