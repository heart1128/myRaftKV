/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-01 17:49:19
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-01 18:03:17
 * @FilePath: /myRaftKv/src/raftRpcProto/raftRpcUtil.cpp
 * @Description: 
 */
#include "raftRpcUtil.h"

#include "rpcChannel.h"
#include "rpcController.h"

RaftRpcUtil::RaftRpcUtil(std::string ip, short port)
{
    // stub传入channel初始化
    m_stub = new raftRpcProtocol::raftRpc_Stub(new RpcChannel(ip, port, true));
}

RaftRpcUtil::~RaftRpcUtil()
{
    delete m_stub;
}

/// @brief 心跳或者日志同步请求
/// @param args 
/// @param response 
/// @return rpc过程是否有错
bool RaftRpcUtil::AppendEntries(raftRpcProtocol::AppendEntriesArgs *args, raftRpcProtocol::AppendEntriesReply *response)
{
    MprpcController controller;
    m_stub->AppendEntries(&controller, args, response, nullptr);
    return !controller.Failed();
}

/// @brief leader快照请求发送
/// @param args 
/// @param response 
/// @return 
bool RaftRpcUtil::InstallSnapshot(raftRpcProtocol::InstallSnapshotRequest *args, raftRpcProtocol::InstallSnapshotResponse *response)
{
    MprpcController controller;
    m_stub->InstallSnapshot(&controller, args, response, nullptr);
    return !controller.Failed();
}

/// @brief candadite发起投票
/// @param args 
/// @param response 
/// @return 
bool RaftRpcUtil::RequestVote(raftRpcProtocol::RequestVoteArgs *args, raftRpcProtocol::RequestVoteReply *response)
{
    MprpcController controller;
    m_stub->RequestVote(&controller, args, response, nullptr);
    return !controller.Failed();
}
