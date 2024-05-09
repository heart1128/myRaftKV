/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-09 13:35:29
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-09 15:52:54
 * @FilePath: /myRaftKv/src/raftClerk/include/clerk.h
 * @Description: 
 */
#ifndef SRC_RAFTCLERK_INCLUDE_CLERK_H
#define SRC_RAFTCLERK_INCLUDE_CLERK_H

#include <vector>
#include <string>

#include "raftServerRpcUtil.h"
#include "rpcConfig.h"

class Clerk
{
public:
    Clerk();

public:
    void Init(std::string configFileName);
    std::string Get(std::string key);

    void Put(std::string, std::string value);
    void Append(std::string, std::string value);

private:
    /// @brief 随机生成clientID
    /// @return     返回客户端唯一标识
    std::string uuid()
    {
        std::string uuid;
        for(int i = 0; i < 4; ++i)
        {
            uuid += std::to_string(rand());
        }
        return uuid;
    }

    void PutAppend(std::string key, std::string value, std::string op);

private:
    std::vector<std::shared_ptr<raftServerRpcUtil>> m_servers;  // 和raft节点的通信，一个客户端可以和任意raft节点通信，前提是leader
    std::string m_clientId;     // 客户端的唯一ID
    int m_requestId;
    int m_recentLeaderID;      //记录上次的leader id，请求就发送到这个id节点
};



#endif // SRC_RAFTCLERK_INCLUDE_CLERK_H