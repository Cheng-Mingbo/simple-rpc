//
// Created by cheng on 23-5-25.
//
#include <iostream>
#include <mutex>
#include "config.h"

namespace rpc {

std::shared_ptr<Config> Config::instance_ = nullptr;

Config::Config(const std::string &yaml_file) {
    try {
        config_ = YAML::LoadFile(yaml_file);
    } catch (const YAML::BadFile& e) {
        std::cerr << "Error: Unable to open config file: " <<  yaml_file << std::endl;
        exit(EXIT_FAILURE);
    } catch (const YAML::ParserException& e) {
        std::cerr << "Error: Unable to parse config file: " << yaml_file << std::endl;
        exit(EXIT_FAILURE);
    }
    log_level_ = get<std::string>("log_level");
    log_path_ = get<std::string>("log_path");
    log_file_name_ = get<std::string>("log_file_name");
    log_file_size_ = get<int>("log_file_size");
    log_sync_interval_ = get<int>("log_sync_interval");
    port_ = get<int>("port");
    io_threads_ = get<int>("io_threads");
}

Config& Config::GetGlobalConfig() {
    return *instance_;
}

void Config::SetGlobalConfig(const std::string &yaml_file) {
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        instance_.reset(new Config(yaml_file));
    });
}
}