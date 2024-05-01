/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:19:16
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-01 15:19:55
 * @FilePath: /myRaftKv/src/rpc/rpcChannel.cpp
 * @Description: 
 */
#include "rpcChannel.h"

#include "util.h"
#include "rpcHeader.pb.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <string>

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

/// @brief 
/// @param ip 
/// @param port 
/// @param connectNow 
RpcChannel::RpcChannel(std::string ip, short port, bool connectNow):
m_ip(ip), m_port(port), m_clientFd(-1)
{
    // 1. 不是新连接不用设置
    if(!connectNow)
        return;
    
    // 2. 如果连接失败可以尝试三次连接，后续放在config里设置
    std::string errMsg;
    bool rt = newConnect(ip.c_str(), port, errMsg);

    int tryCount = 3;
    while(!rt && tryCount--)
    {
        std::cout << errMsg << std::endl;
        rt = newConnect(ip.c_str(), port, errMsg);
    }


}

/// @brief 
/// @param method 
/// @param controller 
/// @param request 
/// @param response 
/// @param done 
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method, 
                            google::protobuf::RpcController *controller, 
                            const google::protobuf::Message *request, 
                            google::protobuf::Message *response, 
                            google::protobuf::Closure *done)
{
    // 没有封装客户端，直接连接，然后打包，发送，接收
    // 流程
    // 1. 从request从拿出数据到string
    // 2. 数据组装到string
    // 3. 发送组装的数据
    // 4. 接收服务端的调用结果
    // 5. 从接收的buf中序列化到response
    

    if(m_clientFd == -1)
    {
        // 连接
        // 因为设置的是短连接，初始化连接一次保存ip，现在是短连接
        std::string errMsg;
        bool rt = newConnect(m_ip.c_str(), m_port, errMsg);
        if(!rt)
        {
            DPrintf("[func-MprpcChannel::CallMethod]重连接ip：{%s} port{%d}失败", m_ip.c_str(), m_port);
            // 设置proto错误
            controller->SetFailed(errMsg);
            return;
        }
        else
        {
            DPrintf("[func-MprpcChannel::CallMethod]连接ip：{%s} port{%d}成功", m_ip.c_str(), m_port);
        }

        // 1. proto拿到服务描述,描述能调用Message Factoryc创建message
        // 获得方法，通过反射的
        const google::protobuf::ServiceDescriptor* sd = method->service();
        std::string service_name = sd->name();
        std::string method_name = method->name();

        // 2. 序列化
        // 从request中序列化到string
        std::string args_str;
        if(!request->SerializeToString(&args_str))
        {
            controller->SetFailed("serialize request error!");
            return;
        }

        // proto生成了rpcHeader（自己设置的），拿出来设置参数
        RPC::RpcHeader _rpcHeader;
        _rpcHeader.set_service_name(service_name);
        _rpcHeader.set_method_name(method_name);
        _rpcHeader.set_args_size(args_str.size());

        std::string rpc_header_str;
        if (!_rpcHeader.SerializeToString(&rpc_header_str)) 
        {
            controller->SetFailed("serialize rpc header error!");
            return;
        }

        // 使用protobuf的CodedOutputStream来构建发送的数据流
        std::string send_rpc_str;
        google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
        google::protobuf::io::CodedOutputStream coded_output(&string_output);

        // 写入header
        coded_output.WriteVarint32(static_cast<uint32_t>(rpc_header_str.size()));

        coded_output.WriteString(rpc_header_str);

        send_rpc_str += rpc_header_str;


        // 发送网络信息
        // 发送失败会一直连接 + 发送
        while(-1 == send(m_clientFd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
        {
            char errtxt[512] = {0};
            sprintf(errtxt, "send error! errno:%d", errno);
            std::cout << "尝试重新连接，对方ip：" << m_ip << " 对方端口" << m_port << std::endl;
            close(m_clientFd);
            m_clientFd = -1;
            std::string errMsg;
            bool rt = newConnect(m_ip.c_str(), m_port, errMsg);
            if (!rt) {
            controller->SetFailed(errMsg);
            return;
            } 
        }


        // 3. 接收数据
        // 发送之后服务端就处理rpc请求了，请求之后循环接收
        char recv_buf[1024] = {0};
        int recv_size = 0;

        if(-1 == (recv_size = recv(m_clientFd, recv_buf, sizeof(recv_buf), 0)))
        {
            close(m_clientFd);
            m_clientFd = -1;
            char errtxt[512] = {0};
            sprintf(errtxt, "recv error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }

        // 4. 反序列化到response
        if(!response->ParseFromArray(recv_buf, recv_size))
        {
            char errtxt[1050] = {0};
            sprintf(errtxt, "parse error! response_str:%s", recv_buf);
            controller->SetFailed(errtxt);
            return;
        }

    }
}

/// @brief 
/// @param ip 
/// @param port 
/// @param errMsg 
/// @return 
bool RpcChannel::newConnect(const char *ip, uint16_t port, std::string& errMsg)
{
    //1. 创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == clientfd)
    {
        errMsg = "create socket error! errno:" + errno;
        m_clientFd = -1;
        return false;
    }

    // 2. connect
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int rt = connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if(-1 == rt)
    {
        close(clientfd);
        errMsg = "connect fail! error! errno:" + errno;
        m_clientFd = -1;
        return false;
    }

    m_clientFd = clientfd;
    return true;
}
