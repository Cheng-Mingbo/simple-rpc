//
// Created by cheng on 23-5-28.
//
#include <cstring>
#include <utility>
#include "net_addr.h"
#include "logger.h"

namespace rpc {
IPNetAddr::IPNetAddr(std::string ip, uint16_t port) : ip_(std::move(ip)), port_(port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &addr_.sin_addr);
}

IPNetAddr::IPNetAddr(const std::string &addr) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    int pos = addr.find(':');
    if (pos == std::string::npos) {
        LOG_ERROR("invalid addr: %s", addr.c_str());
        exit(1);
    }
    ip_ = addr.substr(0, pos);
    port_ = static_cast<uint16_t>(std::stoi(addr.substr(pos + 1)));
    addr_.sin_port = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &addr_.sin_addr);
}

IPNetAddr::IPNetAddr(const sockaddr_in &addr) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = addr.sin_port;
    addr_.sin_addr = addr.sin_addr;
    ip_ = inet_ntoa(addr_.sin_addr);
    port_ = ntohs(addr_.sin_port);
}

sockaddr *IPNetAddr::getSockAddr() const {
    return const_cast<sockaddr *>(reinterpret_cast<const sockaddr *>(&addr_));
}

socklen_t IPNetAddr::getAddrLen() const {
    return sizeof(addr_);
}

int IPNetAddr::getFamily() {
    return AF_INET;
}

std::string IPNetAddr::toString() {
    return ip_ + ":" + std::to_string(port_);
}

bool IPNetAddr::checkValid() {
    if (ip_.empty() || port_ == 0) {
        return false;
    }
    if (port_ < 0 || port_ > 65535) {
        return false;
    }
    if (inet_addr(ip_.c_str()) == INADDR_NONE) {
        return false;
    }
    return true;
}


}