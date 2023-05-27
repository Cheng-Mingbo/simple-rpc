//
// Created by cheng on 23-5-27.
//

#include "wakeup_channel.h"
#include "logger.h"
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace rpc {
void WakeupChannel::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("WakeupChannel::wakeup() error: %d, info[%s]", errno, strerror(errno));
    }
    LOG_DEBUG("WakeupChannel::wakeup() success");
}
}