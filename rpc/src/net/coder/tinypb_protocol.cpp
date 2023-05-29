//
// Created by cheng on 23-5-29.
//

#include "tinypb_protocol.h"

namespace rpc {
char TinyPBProtocol::PB_START_FLAG = 0x02;
char TinyPBProtocol::PB_END_FLAG = 0x03;

std::string TinyPBProtocol::toString() {
    std::string ret;
    ret += "package_len: " + std::to_string(package_len_) + "\n";
    ret += "msg_id_len: " + std::to_string(msg_id_len_) + "\n";
    ret += "msg_id: " + msg_id_ + "\n";
    ret += "method_name_len: " + std::to_string(method_name_len_) + "\n";
    ret += "method_name: " + method_name_ + "\n";
    ret += "err_code: " + std::to_string(err_code_) + "\n";
    ret += "err_msg_len: " + std::to_string(err_msg_len_) + "\n";
    ret += "err_msg: " + err_msg_ + "\n";
    ret += "pb_data: " + pb_data_ + "\n";
    ret += "check_sum: " + std::to_string(check_sum_) + "\n";
    return ret;
}
}