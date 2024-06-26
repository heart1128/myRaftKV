/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 20:16:39
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-03 14:07:42
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

class RpcProvider{
public:
    // 事件注册，其实就是放在一个map中
    void NotifyService(google::protobuf::Service* service);

    // 启动节点，创建服务端用的muduo
    void Run(int nodeIndex, short port);

    ~RpcProvider();

private:
    // epoll
    // 这个移动到util类了，因为raft也想用里面的定时器事件
    //  muduo::net::EventLoop m_eventLoop;
    // tcp server

    muduo::net::EventLoop eventLoop;
    std::shared_ptr<muduo::net::TcpServer> m_ptr_muduo_server;

    struct ServiceInfo{
        google::protobuf::Service *m_service; // 服务
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 方法注册中心,一个service可能多个方法
    };
    // 保存服务信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // socker回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 读写回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    // closure回调，rpc发送最后清理的时候调用这个
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);


};


#endif // SRC_RPC_INCLUDE_RPCPROVIDER_H