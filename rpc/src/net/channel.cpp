//
// Created by cheng on 23-5-27.
//

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "channel.h"


namespace rpc {

Channel::Channel(int fd) : fd_(fd) {
    bzero(&events_, sizeof(events_));
}

Channel::Channel() {
    bzero(&events_, sizeof(events_));
}


void Channel::setNonBlock() {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags & O_NONBLOCK) {
        return;
    }
    flags |= O_NONBLOCK;
    fcntl(fd_, F_SETFL, flags);
}

std::function<void()> Channel::handler(Channel::TriggerEvent event) {
    if (event == TriggerEvent::kReadEvent) {
        return read_callback_;
    } else if (event == TriggerEvent::kWriteEvent) {
        return write_callback_;
    } else if (event == TriggerEvent::kErrorEvent) {
        return error_callback_;
    }
    return nullptr;
}

void
Channel::listen(Channel::TriggerEvent event, std::function<void()> callback, std::function<void()> error_callback) {
    if (event == TriggerEvent::kReadEvent) {
        events_.events |= EPOLLIN;
        read_callback_ = callback;
    } else if (event == TriggerEvent::kWriteEvent) {
        events_.events |= EPOLLOUT;
        write_callback_ = callback;
    }
    
    if (error_callback != nullptr) {
        events_.events |= EPOLLERR;
        error_callback_ = error_callback;
    }
    
    events_.data.ptr = this;
}

void Channel::cancel(Channel::TriggerEvent event) {
    if (event == TriggerEvent::kReadEvent) {
        events_.events &= ~EPOLLIN;
        read_callback_ = nullptr;
    } else if (event == TriggerEvent::kWriteEvent) {
        events_.events &= ~EPOLLOUT;
        write_callback_ = nullptr;
    } else if (event == TriggerEvent::kErrorEvent) {
        events_.events &= ~EPOLLERR;
        error_callback_ = nullptr;
    }
}

void Channel::close() {
   ::close(fd_);
}
}