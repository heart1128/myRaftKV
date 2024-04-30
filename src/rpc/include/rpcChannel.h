/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:10:43
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-04-30 18:26:18
 * @FilePath: /myRaftKv/src/rpc/include/rpcChannel.h
 * @Description: 
 */
#ifndef SRC_RPC_INCLUDE_RPCCHANNEL_H
#define SRC_RPC_INCLUDE_RPCCHANNEL_H

// 这个文件才是重点，实现CallMethod方法
// 1. 组装序列化 2.发送 3. 接收

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <string>

// 这个类处理了连接
class RpcChannel : public google::protobuf::RpcChannel{
public:
    RpcChannel(std::string ip, short port, bool connectNow);
public:
    // 重新写CallMethod
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done) override;
    

private:
    int m_clientFd; // 客户端连接的fd
    const std::string m_ip;  // ip和端口
    const uint16_t m_port; // 端口

    // 处理连接
    bool newConnect(const char* ip, uint16_t port, std::string& errMsg);
};


#endif