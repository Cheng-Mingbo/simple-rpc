//
// Created by cheng on 23-5-27.
//

#ifndef RPC_CHANNEL_H
#define RPC_CHANNEL_H
#include <functional>
#include <sys/epoll.h>

namespace rpc {
class Channel {
  public:
    using read_callback = std::function<void()>;
    using write_callback = std::function<void()>;
    using error_callback = std::function<void()>;
    
    enum class TriggerEvent {
        kReadEvent = EPOLLIN,
        kWriteEvent = EPOLLOUT,
        kErrorEvent = EPOLLERR,
        kNoneEvent = 0
    };
    
    explicit Channel(int fd);
    Channel();
    virtual ~Channel() = default;
    
    void setNonBlock();
    std::function<void()> handler(TriggerEvent event);
    void listen(TriggerEvent event, std::function<void()> callback, std::function<void()> error_callback = nullptr);
    void cancel(TriggerEvent event);
    
    int getFd() const { return fd_; }
    epoll_event getEvents() { return events_; }
    
    void close();
    
  protected:
    int fd_ {-1};
    epoll_event events_;
    read_callback read_callback_;
    write_callback write_callback_;
    error_callback error_callback_;
};


}
#endif //RPC_CHANNEL_H
