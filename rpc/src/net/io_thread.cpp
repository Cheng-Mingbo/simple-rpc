//
// Created by cheng on 23-5-27.
//

#include "io_thread.h"
#include "logger.h"
#include "util.h"


namespace rpc {
IOThread::IOThread() : init_sem_(0), start_sem_(0) {
    thread_ = std::jthread([this] {rpc::IOThread::threadFunc(this);});
    init_sem_.acquire();
    LOG_DEBUG("IOThread init success, tid=%d", tid_);
}

IOThread::~IOThread() {
    event_loop_->stop();
    
    delete event_loop_;
    event_loop_ = nullptr;
    LOG_DEBUG("IOThread destroy, tid=%d", tid_);
}

void IOThread::threadFunc(IOThread* io_thread) {
    io_thread->event_loop_ = new EventLoop();
    io_thread->tid_ = getThreadId();
    
    io_thread->init_sem_.release();
    
    io_thread->start_sem_.acquire();
    LOG_DEBUG("IOThread start, tid=%d", io_thread->tid_);
    io_thread->event_loop_->loop();
}

void IOThread::start() {
    start_sem_.release();
}

void IOThread::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}


}