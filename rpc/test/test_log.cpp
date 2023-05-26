//
// Created by cheng on 23-5-26.
//
#include "logger.h"
#include "config.h"
#include <thread>


int main() {
    rpc::Config::SetGlobalConfig("config.yaml");
    rpc::Logger::GetGlobalLogger();
    std::jthread give_me_a_name([] {
        for (int i = 0; i < 100000; ++i) {
            LOG_INFO("hello world %d", i);
            rpc::Logger::GetGlobalLogger().syncLoop();
        }
    });
    for (int i = 0; i < 100000; ++i) {
        LOG_INFO("hello world %d", i);
    }
    
    
    std::this_thread::sleep_for(std::chrono::seconds(12));
    rpc::Logger::GetGlobalLogger().syncLoop();
    
    std::cout << "Hello, World!" << std::endl;
    return 0;
}