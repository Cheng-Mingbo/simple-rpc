//
// Created by cheng on 23-5-25.
//

#ifndef RPC_FILE_APPENDER_H
#define RPC_FILE_APPENDER_H
#include "logevent.h"
#include "appender.h"
#include <fstream>
#include <ios>
#include <vector>
#include <mutex>

namespace rpc {
class FileAppender : public Appender {
  public:
    explicit FileAppender();
    ~FileAppender() override = default;
    
    void append(const std::string& event) override;
    void append_app(const std::string& event) override;
    void swap(std::vector<std::vector<std::string>>&) override;
  private:
    std::vector<std::string> buffer_;
    std::vector<std::string> app_buffer_;
    std::mutex buffer_mutex_;
    std::mutex app_buffer_mutex_;
};
}
#endif //RPC_FILE_APPENDER_H
