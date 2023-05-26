//
// Created by cheng on 23-5-25.
//
#include <fstream>
#include <utility>
#include "file_appender.h"

namespace rpc {

void FileAppender::append(const std::string &event) {
    std::scoped_lock<std::mutex> lock(buffer_mutex_);
    buffer_.push_back(event);
}

void FileAppender::append_app(const std::string &event) {
    std::scoped_lock<std::mutex> lock(app_buffer_mutex_);
    app_buffer_.push_back(event);
}

void FileAppender::swap(std::vector<std::vector<std::string>> & vecs) {
    std::scoped_lock<std::mutex> lock(buffer_mutex_);
    std::scoped_lock<std::mutex> lock2(app_buffer_mutex_);
    vecs[0].swap(buffer_);
    vecs[1].swap(app_buffer_);
}



FileAppender::FileAppender() = default;

}