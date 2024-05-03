#ifndef SRC_RAFTRPCPROTO_INCLUDE_RAFTRPC_H
#define SRC_RAFTRPCPROTO_INCLUDE_RAFTRPC_H

#include "raftRPC.pb.h"

/// @brief 这个类就是实现三个rpc server的调用发送，所以这里可以看做是客户端
//          要定义stub，然后发送请求
class RaftRpcUtil{
private:
    raftRpcProtocol::raftRpc_Stub* m_stub;

public:
    /// @brief 要发送给谁就传入谁的ip port
    /// @param ip 
    /// @param port 
    RaftRpcUtil(std::string ip, short port);
    ~RaftRpcUtil();

public:
    // 三个发送请求的函数
    bool AppendEntries(raftRpcProtocol::AppendEntriesArgs *args, raftRpcProtocol::AppendEntriesReply *response);
    bool InstallSnapshot(raftRpcProtocol::InstallSnapshotRequest *args, raftRpcProtocol::InstallSnapshotResponse *response);
    bool RequestVote(raftRpcProtocol::RequestVoteArgs *args, raftRpcProtocol::RequestVoteReply *response);
};


#endif // SRC_RAFTRPCPROTO_INCLUDE_RAFTRPC_H