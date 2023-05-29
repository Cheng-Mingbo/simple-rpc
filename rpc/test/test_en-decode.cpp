//
// Created by cheng on 23-5-29.
//
#include "tinypb_protocol.h"
#include "buffer.h"
#include "tinypb_coder.h"
#include <iostream>
#include <vector>


void test_encode_and_decode() {
    auto coder = std::make_shared<rpc::TinyPBCoder>();
    auto message = std::make_shared<rpc::TinyPBProtocol>();
    auto input_buffer = std::make_shared<rpc::TcpBuffer>(1024);
    auto output_buffer = std::make_shared<rpc::TcpBuffer>(1024);
    message->msg_id_ = "1234";
    message->method_name_ = "hello";
    message->err_code_ = 0;
    message->err_msg_ = "success";
    message->pb_data_ = "hello world";
    
    std::vector<rpc::AbstractProtocol::s_ptr> messages;
    messages.push_back(message);
    coder->encode(messages, output_buffer);
    output_buffer->writeToBuffer("1234567890", 10);
    message->msg_id_ = "5678";
    coder->encode(messages, output_buffer);
    output_buffer->moveWriteIndex(-1);
    std::vector<rpc::AbstractProtocol::s_ptr> messages2;
    coder->decode(messages2, output_buffer);
    
   for (auto& msg : messages2) {
         auto tiny_pb_message = std::dynamic_pointer_cast<rpc::TinyPBProtocol>(msg);
         std::cout << "tiny_pb_message->msg_id_ = " << tiny_pb_message->msg_id_ << std::endl;
         std::cout << "tiny_pb_message->method_name_ = " << tiny_pb_message->method_name_ << std::endl;
         std::cout << "tiny_pb_message->err_code_ = " << tiny_pb_message->err_code_ << std::endl;
         std::cout << "tiny_pb_message->err_msg_ = " << tiny_pb_message->err_msg_ << std::endl;
         std::cout << "tiny_pb_message->pb_data_ = " << tiny_pb_message->pb_data_ << std::endl;
   }
   std::cout << std::endl;
   output_buffer->moveWriteIndex(1);
   messages2.clear();
   coder->decode(messages2, output_buffer);
    for (auto& msg : messages2) {
        auto tiny_pb_message = std::dynamic_pointer_cast<rpc::TinyPBProtocol>(msg);
        std::cout << "tiny_pb_message->msg_id_ = " << tiny_pb_message->msg_id_ << std::endl;
        std::cout << "tiny_pb_message->method_name_ = " << tiny_pb_message->method_name_ << std::endl;
        std::cout << "tiny_pb_message->err_code_ = " << tiny_pb_message->err_code_ << std::endl;
        std::cout << "tiny_pb_message->err_msg_ = " << tiny_pb_message->err_msg_ << std::endl;
        std::cout << "tiny_pb_message->pb_data_ = " << tiny_pb_message->pb_data_ << std::endl;
    }
    
}

int main() {
    test_encode_and_decode();
    return 0;
}