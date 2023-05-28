//
// Created by cheng on 23-5-28.
//
#include <memory>
#include "tcp_server.h"
#include "config.h"
#include "logger.h"


void test_tcp_server() {
    
    rpc::IPNetAddr::s_ptr addr = std::make_shared<rpc::IPNetAddr>("127.0.0.1", 8080);
    
    LOG_INFO("create addr %s", addr->toString().c_str());
    
    rpc::TcpServer tcp_server(addr);
    
    tcp_server.start();
    
}

int main() {
    
    rpc::Config::SetGlobalConfig("config.yaml");
    rpc::Logger::InitGlobalLogger();
    
    test_tcp_server();
    
}