//
// Created by cheng on 23-5-25.
//

#ifndef RPC_UTIL_H
#define RPC_UTIL_H
#include <sys/types.h>
#include <unistd.h>

namespace rpc {
pid_t getPid();
pid_t getThreadId();
}
#endif //RPC_UTIL_H
