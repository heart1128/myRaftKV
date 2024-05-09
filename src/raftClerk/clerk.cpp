/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-09 13:35:36
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-09 16:02:19
 * @FilePath: /myRaftKv/src/raftClerk/clerk.cpp
 * @Description: 
 */
#include "clerk.h"
#include "util.h"

Clerk::Clerk()
    :m_clientId(uuid()), m_requestId(0), m_recentLeaderID(0)
{
}

/// @brief 读取node文件，初始化连接
/// @param configFileName 
void Clerk::Init(std::string configFileName)
{
    MpRpcConfig config;
    config.LoadConfigFile(configFileName.c_str());

    std::vector<std::pair<std::string, short>> ipPortVt;

    // 1. 解析node文件，拿到ip port
    for(int i = 0; i < INT_MAX - 1; ++i)
    {
        std::string node = "node" + std::to_string(i);

        std::string ip = config.Load(node + "ip");
        std::string port = config.Load(node + "port");
        if(ip.empty())
        {
            break;
        }

        ipPortVt.emplace_back(ip, atoi(port.c_str()));
    }

    // 2. 连接
    for(const auto& item : ipPortVt)
    {
        std::string ip = item.first;
        short port = item.second;

        auto* rpc = new raftServerRpcUtil(ip, port);
        m_servers.push_back(std::shared_ptr<raftServerRpcUtil>(rpc));
    }
}

/// @brief 用rpc发送请求到kvServer，然后返回结果
/// @param key 
/// @return 返回查询的key->value
std::string Clerk::Get(std::string key)
{
    m_requestId++;      // requestId是递增的，线性一致性
    int requestId = m_requestId;
    int server = m_recentLeaderID;

    raftKVRpcProctoc::GetArgs args;
    args.set_key(key);
    args.set_clientid(m_clientId);
    args.set_requestid(requestId);

    while(true) // 如果失败会重复发，直到成功
    {
        raftKVRpcProctoc::GetReply reply;
        bool ok = m_servers[server]->Get(&args, &reply);    // 请求，调用kvServer的Get

        if(!ok || reply.err() == ErrWrongLeader)            // 当前节点不是leader，换节点
        {
            server = (server + 1) % m_servers.size();
            continue;
        }

        if(reply.err() == ErrNoKey)                         // kv数据库中没有这个key
        {
            return "";
        }

        if(reply.err() == OK)
        {
            m_recentLeaderID = server;
            return reply.value();
        }
    }
    return "";
}

void Clerk::PutAppend(std::string key, std::string value, std::string op)
{
    m_requestId++;

    int requestId = m_requestId;
    int server = m_recentLeaderID;

    while(true)
    {
        raftKVRpcProctoc::PutAppendArgs args;
        args.set_key(key);
        args.set_value(value);
        args.set_clientid(m_clientId);
        args.set_requestid(requestId);
        args.set_op(op);

        raftKVRpcProctoc::PutAppendReply reply;
        bool ok = m_servers[server]->PutAppend(&args, &reply);

        if(!ok || reply.err() == ErrWrongLeader)    // 换节点重试
        {
            DPrintf("【Clerk::PutAppend】原以为的leader：{%d}请求失败，向新leader{%d}重试  ，操作：{%s}", server, server + 1,
              op.c_str());
            
            if(!ok)
            {
                DPrintf("重试原因，rpc失败!");
            }
            if(reply.err() == ErrWrongLeader)       // 非leader，尝试下一个server
            {
                server = (server + 1) % m_servers.size();
            }
            continue;
        }
        else
        {
            if(reply.err() == OK)                   // 1.存入到raft logs 2. 存入到kv中 返回OK
            {
                m_recentLeaderID = server;
                return;
            }
        }
    }
}

void Clerk::Put(std::string key, std::string value)
{
    PutAppend(key, value, "Put");
}

void Clerk::Append(std::string key, std::string value)
{
    PutAppend(key, value, "Append");
}

