//
// Created by cheng on 23-5-27.
//

#include "io_thread_group.h"
#include "io_thread.h"

namespace rpc {
IOThreadGroup::IOThreadGroup(int size) : size_(size) {
    for (int i = 0; i < size_; ++i) {
        io_threads_.push_back(new IOThread());
    }
}

void IOThreadGroup::start() {
    for (auto &io_thread: io_threads_) {
        io_thread->start();
    }
}


IOThread* IOThreadGroup::getNextIOThread() {
    return io_threads_[index_++ % size_];
}

void IOThreadGroup::join() {
    for (auto &io_thread: io_threads_) {
        io_thread->join();
    }
}

}