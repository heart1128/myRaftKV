/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 20:16:39
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-04-30 20:25:26
 * @FilePath: /myRaftKv/src/rpc/include/rpcProvider.h
 * @Description: 
 */
#ifndef SRC_RPC_INCLUDE_RPCPROVIDER_H
#define SRC_RPC_INCLUDE_RPCPROVIDER_H

#include <google/protobuf/descriptor.h>
#include "google/protobuf/service.h"

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>

#include <functional>
#include <string>
#include <unordered_map>

// 提供了三个功能：
// 1. 注册rpc服务（没用zk）
// 2. 建立服务端
// 3. 处理rpc事件完成远程调用

class PpcProvider{
public:
    // 事件注册，其实就是放在一个map中
    void NotifyService(google::protobuf::Service* service);

    // 启动节点，创建服务端用的muduo
    void Run(int nodeIndex, short port);

private:
    // epoll
    muduo::net::EventLoop m_eventLoop;
    // tcp server
    std::shared_ptr<muduo::net::TcpServer> m_ptr_muduo_server;

    struct ServiceInfo{
        google::protobuf::Service *m_service; // 服务
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 方法注册中心,一个service可能多个方法
    };
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;



};


#endif // SRC_RPC_INCLUDE_RPCPROVIDER_H