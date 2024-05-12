#include "rpcProvider.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <string>
#include "rpcHeader.pb.h"
#include "util.h"
#include "config.h"


/// @brief 添加服务通知，简单使用了一个map添加服务名和方法
/// @param service 
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 通过DescriptorPool获取服务对象的描述信息
    // 反射实现，包含name(), full_name()
    const google::protobuf::ServiceDescriptor* pServiceDesc = service->GetDescriptor();
    // 服务名
    std::string service_name = pServiceDesc->name();
    // 获取方法数量，然后通过循环放入结构体中
    int methodCnt = pServiceDesc->method_count();

    std::cout << "service_name:" << service_name << std::endl;

    for(int i = 0; i < methodCnt; ++i)
    {
        // 获取方法，方法是有编号的
        const google::protobuf::MethodDescriptor* pMethodDesc = pServiceDesc->method(i);
        std::string method_name = pMethodDesc->name();
        // 加入结构体中的methodmap
        service_info.m_methodMap[method_name] = pMethodDesc;
    }

    // 加入服务
    service_info.m_service = service;
    m_serviceMap[service_name] = service_info;
}

/// @brief 拿到节点的ip：port，启动server，处理服务端的proto序列化，填写发送
/// @param nodeIndex  raft节点的序号
/// @param port  端口
void RpcProvider::Run(int nodeIndex, short port)
{
    // 1. 获取本机的所有ip，因为一个server是一个台机器
    char *ipC;
    char hostname[128];
    struct hostent* hent;
    gethostname(hostname, sizeof(hostname));     // 拿出本机主机名
    hent = gethostbyname(hostname);  // 拿出主机名所有的网络信息
        // 获取所有ip地址h_addr_list是一个二级指针
    for(int i = 0; hent->h_addr_list[i]; ++i)
    {
        ipC = inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i]));
    }
    std::string ip = std::string(ipC);

    // 2. 将本机所有ip:port写入文件
    std::string node = "node" + std::to_string(nodeIndex);
    std::ofstream outfile;
    outfile.open("node.conf", std::ios::app);
    if(!outfile.is_open())
    {
        std::cout << "文件打开失败!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // node1ip=xxxxx node1port=xxxx
    outfile << node + "ip=" + ip << std::endl;
    outfile << node + "port=" + std::to_string(port) << std::endl;
    outfile.close();


    // 3. 启动server
    DPrintf("当前进程: %d, 当前线程：%d, server启动在 ip = %s, port = %d", getpid(), gettid() ,ip.c_str(), port);
    muduo::net::InetAddress address(ip, port);

    m_ptr_muduo_server = std::make_shared<muduo::net::TcpServer>(&eventLoop, address, "RpcProvider");

    // 连接回调, 还不知道传什么进去，按照函数参数占位
    m_ptr_muduo_server->setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    // io回调，这里是处理消息的主要函数
    m_ptr_muduo_server->setMessageCallback(std::bind(&RpcProvider::OnMessage, this, 
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置线程数量 + 启动server + 启动epoll
    m_ptr_muduo_server->setThreadNum(4);

    m_ptr_muduo_server->start();
    eventLoop.loop();
}

RpcProvider::~RpcProvider()
{
    std::cout << "[func - RpcProvider::~RpcProvider()]: ip和port信息：" << m_ptr_muduo_server->ipPort() << std::endl;
    eventLoop.quit(); // 整个服务停止
}

/// @brief 这里是tpcconn连接之前的时间点，这个函数主要就是处理一下中间事件
/// @param  conn tpc连接对象
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    // 没有连接，不用做什么，等待tcpconn连接
    if(!conn->connected())
    {
        // 旧连接先断开，再连接新的。
        conn->shutdown();
    }
}

/// @brief 触发有io事件就执行这个回调函数，proto反序列化，填写，发送
/// @param conn 已连接对象
/// @param buffer io buffer，已经读取的buffer
/// @param  
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
    std::cout << __LINE__ << "server接收到的数据长度 : " << buffer->readableBytes() << std::endl;
    std::string recv_buf = buffer->retrieveAllAsString(); // 读取所有内容，刷新buffer

    // // 使用protobuf的CodedInputStream来解析数据流
    google::protobuf::io::ArrayInputStream array_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&array_input);
    uint32_t header_size{};

    coded_input.ReadVarint32(&header_size);
    // 这块就是客户端的反向操作
    std::string rpc_header_str;
    RPC::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;

    // 设置读取限制，不必担心数据读多
    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    // 读取rpc内容
    coded_input.ReadString(&rpc_header_str, header_size);
    coded_input.PopLimit(msg_limit); // 恢复限制，其他数据不一定只读这个大小

    uint32_t args_size{};

    // 反序列化
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else // 反序列化失败
    {
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str;
    // 直接读取args_size长度的字符串数据
    bool read_args_success = coded_input.ReadString(&args_str, args_size);

    if (!read_args_success) 
    {
        // 处理错误：参数数据读取失败
        return;
    }


    // 处理服务, 找到server对象和method对象
    auto serviceIt = m_serviceMap.find(service_name);
    if(serviceIt == m_serviceMap.end())  // 没有这个服务
    {
        std::cout << "服务：" << service_name << " is not exist!" << std::endl;
        std::cout << "当前已经有的服务列表为:";
        for (auto item : m_serviceMap) {
        std::cout << item.first << " ";
        }
        std::cout << std::endl;
        return;
    }

    // 找到方法
    auto methodIt = serviceIt->second.m_methodMap.find(method_name);
    if(methodIt == serviceIt->second.m_methodMap.end()) // 没有这个方法
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    // 执行方法
    google::protobuf::Service* service = serviceIt->second.m_service;
    const google::protobuf::MethodDescriptor* method = methodIt->second;

    // 生成请求和响应
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))  // 解析内容到rquest
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }

    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // response要等到执行了新继承Service的类实现，内部会根据方法找到重载的实现
    // server的CallMethod和客户端的不一样，客户端是通过stub调用,服务端是继承实现调用

    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(
          this, &RpcProvider::SendRpcResponse, conn, response);
    
    // 重点，调用方法，实现response的填充
    // 这个函数最后会调用done，这里done函数就是SendRpcResponse进行发送
    service->CallMethod(method, nullptr, request, response, done);
}

/// @brief 这个函数在service->CallMethod最后被调用，执行发送到客户端
/// @param conn 发送的连接对象
/// @param response 填充好的response
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    // 序列化到string发送
    if(response->SerializeToString(&response_str))
    {
        std::cout << __LINE__ << "server发送长度：" << response_str.size() << std::endl;
        conn->send(response_str); // 直接发送
    }
    else // 序列化失败
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
}
