//
// Created by cheng on 23-5-27.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <memory>
#include "logger.h"
#include "config.h"
#include "eventloop.h"
#include "io_thread.h"
#include "io_thread_group.h"

void test_io_thread() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        LOG_ERROR("listenfd = -1");
        exit(0);
    }
    
    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));
    
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);
    
    int rt = bind(listenfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (rt != 0) {
        LOG_ERROR("bind error");
        exit(1);
    }
    
    rt = listen(listenfd, 100);
    if (rt != 0) {
        LOG_ERROR("listen error");
        exit(1);
    }
    
    auto* event = new rpc::Channel(listenfd);
    event->listen(rpc::Channel::TriggerEvent::kReadEvent, [listenfd]() {
        sockaddr_in peer_addr{};
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr *>(&peer_addr), &addr_len);
        
        LOG_DEBUG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr),
                  ntohs(peer_addr.sin_port));
        
    });
    
    int i = 0;
    rpc::TimerChannel::s_ptr timer_event = std::make_shared<rpc::TimerChannel>(
            1000, true, [&i]() {
                LOG_INFO("trigger timer event, count=%d", i++);
            }
    );
    
    rpc::IOThreadGroup io_thread_group(2);
    rpc::IOThread* io_thread = io_thread_group.getNextIOThread();
    io_thread->getEventLoop()->addEpollEvent(event);
    //io_thread->getEventLoop()->addTimer(timer_event);
    
    rpc::IOThread* io_thread2 = io_thread_group.getNextIOThread();
    // io_thread2->getEventLoop()->addTimer(timer_event);
    
    io_thread_group.start();
}

int main() {
    
    rpc::Config::SetGlobalConfig("config.yaml");
    
    rpc::Logger::SetGlobalLogger();
    
    test_io_thread();
    
     auto* eventloop = rpc::EventLoop::getEventLoopOfCurrentThread();
     eventloop->loop();
//    auto* eventloop = rpc::EventLoop::getEventLoopOfCurrentThread();
//     int listenfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (listenfd == -1) {
//       LOG_ERROR("listenfd = -1");
//       exit(0);
//     }
//
//     sockaddr_in addr{};
//     memset(&addr, 0, sizeof(addr));
//
//     addr.sin_port = htons(8080);
//     addr.sin_family = AF_INET;
//     inet_aton("127.0.0.1", &addr.sin_addr);
//
//     int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
//     if (rt != 0) {
//       LOG_ERROR("bind error");
//       exit(1);
//     }
//
//     rt = listen(listenfd, 100);
//     if (rt != 0) {
//       LOG_ERROR("listen error");
//       exit(1);
//     }
//
//     rpc::Channel event(listenfd);
//     event.listen(rpc::Channel::TriggerEvent::kReadEvent, [listenfd](){
//       sockaddr_in peer_addr{};
//       socklen_t addr_len = sizeof(peer_addr);
//       memset(&peer_addr, 0, sizeof(peer_addr));
//       int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);
//
//       LOG_DEBUG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
//
//     });
//     eventloop->addEpollEvent(&event);
//
//     int i = 0;
//     rpc::TimerChannel::s_ptr timer_event = std::make_shared<rpc::TimerChannel>(
//       1000, true, [&i, &timer_event]() {
//         LOG_INFO("trigger timer event, count=%d", i++);
//         if (i == 10) {
//             timer_event->setCancel(true);
//         }
//       }
//     );
//
//     eventloop->addTimer(timer_event);
//
//
//     eventloop->loop();
//
    return 0;
}