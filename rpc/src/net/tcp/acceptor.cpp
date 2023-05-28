//
// Created by cheng on 23-5-28.
//

#include "acceptor.h"
#include "logger.h"
#include <utility>
#include <sys/socket.h>
#include <cstring>
#include <fcntl.h>


namespace rpc {

TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr) : local_addr_(std::move(local_addr)){
    if (!local_addr_->checkValid()) {
        LOG_ERROR("invalid addr: %s", local_addr_->toString().c_str());
        exit(1);
    }
    family_ = local_addr_->getFamily();
    listen_fd_ = socket(family_, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        LOG_ERROR("create socket error: %s", strerror(errno));
        exit(1);
    }
    int val = 1;
    int ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (ret < 0) {
        LOG_ERROR("setsockopt error: %s", strerror(errno));
    }
    
//    // set non-blocking
//    int flags = fcntl(listen_fd_, F_GETFL, 0);
//    if (flags < 0) {
//        LOG_ERROR("fcntl error: %s", strerror(errno));
//    }
//    flags |= O_NONBLOCK;
//    ret = fcntl(listen_fd_, F_SETFL, flags);
//    if (ret < 0) {
//        LOG_ERROR("fcntl error: %s", strerror(errno));
//    }
    
    socklen_t addr_len = local_addr_->getAddrLen();
    ret = bind(listen_fd_, local_addr_->getSockAddr(), addr_len);
    if (ret < 0) {
        LOG_ERROR("bind error: %s", strerror(errno));
        exit(1);
    }
    ret = listen(listen_fd_, 1024);
    if (ret < 0) {
        LOG_ERROR("listen error: %s", strerror(errno));
        exit(1);
    }
}

std::pair<int, NetAddr::s_ptr>TcpAcceptor::accept() {
    if (family_ == AF_INET) {
        sockaddr_in addr{};
        bzero(&addr, sizeof(addr));
        socklen_t addr_len = sizeof(addr);
        int fd = ::accept(listen_fd_, reinterpret_cast<sockaddr *>(&addr), &addr_len);
        if (fd < 0) {
            LOG_ERROR("accept error: %s", strerror(errno));
            return {-1, nullptr};
        }
        IPNetAddr::s_ptr net_addr = std::make_shared<IPNetAddr>(addr);
        LOG_INFO("accept a connection from %s", net_addr->toString().c_str());
        return {fd, net_addr};
    } else {
        LOG_ERROR("unknown family: %d", family_);
        return {-1, nullptr};
    }
}
}