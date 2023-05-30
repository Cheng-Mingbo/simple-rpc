//
// Created by cheng on 23-5-30.
//

#include "run_time.h"
#include <mutex>

namespace rpc {
thread_local RunTime::s_ptr g_run_time = nullptr;


RunTime::s_ptr RunTime::GetRunTime() {
    static std::once_flag flag;
    if (g_run_time == nullptr) {
        std::call_once(flag, [&]() {
            g_run_time.reset(new RunTime());
        });
    }
    return g_run_time;
}
}
