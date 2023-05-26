//
// Created by cheng on 23-5-25.
//

#include "console_appender.h"

namespace rpc {
void ConsoleAppender::append(const std::string &event) {
    os_ << event;
}

void ConsoleAppender::append_app(const std::string &event) {
    os_ << event;
}

void ConsoleAppender::swap(std::vector<std::vector<std::string>> &) {

}
}