//
// Created by cheng on 23-5-25.
//
#ifndef RPC_CONFIG_H
#define RPC_CONFIG_H
#include <string>
#include <iostream>
#include <yaml-cpp/yaml.h>
namespace rpc {
class Config {
  public:
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    template <typename T>
    T get(const std::string& key) const {
        try {
            return config_[key].as<T>();
        } catch (const YAML::BadConversion& e) {
            std::cerr << "Error: Unable to convert value for key: " << key << std::endl;
            exit(EXIT_FAILURE);
        }
    }
  public:
    static Config& GetGlobalConfig();
    static void SetGlobalConfig(const std::string& yaml_file);
  
  public:
    std::string log_level_;
    std::string log_path_;
    std::string log_file_name_;
    int log_file_size_{0};
    int log_sync_interval_{0};
    
    int port_{0};
    int io_threads_{0};
    
  private:
    explicit Config(const std::string& yaml_file);
    Config() = default;
    static std::shared_ptr<Config> instance_;
    YAML::Node config_;
};
}
#endif //RPC_CONFIG_H
