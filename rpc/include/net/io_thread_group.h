//
// Created by cheng on 23-5-27.
//

#ifndef RPC_IO_THREAD_GROUP_H
#define RPC_IO_THREAD_GROUP_H
#include <vector>
#include <atomic>

namespace rpc {
class IOThread;
class IOThreadGroup {
  public:
    explicit IOThreadGroup(int size);
    ~IOThreadGroup() = default;
    
    void start();
    void join();
    
    IOThread* getNextIOThread();
    
  private:
    int size_{0};
    std::vector<IOThread*> io_threads_;
    std::atomic<int> index_{0};
};
}
#endif //RPC_IO_THREAD_GROUP_H
