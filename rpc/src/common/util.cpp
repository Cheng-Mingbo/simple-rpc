//
// Created by cheng on 23-5-25.
//
#include <sys/syscall.h>
#include <semaphore>
#include "util.h"


namespace rpc {
static thread_local int t_id = 0;
static int g_pid = 0;

pid_t getPid() {
    if (g_pid == 0) {
        g_pid = getpid();
    }
    return g_pid;
}

pid_t getThreadId() {
    if (t_id == 0) {
        t_id = static_cast<int>(syscall(SYS_gettid));
    }
    return t_id;
}

int64_t getNowTime() {
    struct timeval now_time{ };
    gettimeofday(&now_time, nullptr);
    return now_time.tv_sec * 1000 + now_time.tv_usec / 1000;
}
}