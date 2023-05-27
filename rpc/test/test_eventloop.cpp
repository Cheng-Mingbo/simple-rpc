//
// Created by cheng on 23-5-27.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <memory>
#include "logger.h"
#include "config.h"
#include "eventloop.h"

int main() {
    
    rpc::Config::SetGlobalConfig("config.yaml");
    
    rpc::Logger::GetGlobalLogger();
    
    //test_io_thread();
    
     rpc::EventLoop* eventloop = new rpc::EventLoop();
    
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
    
     int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
     if (rt != 0) {
       LOG_ERROR("bind error");
       exit(1);
     }
    
     rt = listen(listenfd, 100);
     if (rt != 0) {
       LOG_ERROR("listen error");
       exit(1);
     }
     
     rpc::Channel event(listenfd);
     event.listen(rpc::Channel::TriggerEvent::kReadEvent, [listenfd](){
       sockaddr_in peer_addr{};
       socklen_t addr_len = sizeof(peer_addr);
       memset(&peer_addr, 0, sizeof(peer_addr));
       int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);
    
       LOG_DEBUG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    
     });
     eventloop->addEpollEvent(&event);
    
     int i = 0;
     rpc::TimerChannel::s_ptr timer_event = std::make_shared<rpc::TimerChannel>(
       1000, true, [&i, &timer_event]() {
         LOG_INFO("trigger timer event, count=%d", i++);
         if (i == 10) {
             timer_event->setCancel(true);
         }
       }
     );
    
     eventloop->addTimer(timer_event);
     eventloop->loop();
    
    return 0;
}