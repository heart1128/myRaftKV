/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-02 15:14:47
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-02 15:20:12
 * @FilePath: /myRaftKv/src/raftRpcProto/include/applyMsg.h
 * @Description: 
 */
#ifndef SRC_RAFTRPCPROTO_INCLUDE_APPLYMSG_H
#define SRC_RAFTRPCPROTO_INCLUDE_APPLYMSG_H

#include <string>

/// @brief 这个类是client和raft进行通信的消息结构
class ApplyMsg
{
public:
    bool CommandValid; // clinet发送的请求指令是否有效 k v
    std::string Command; // 发送的消息指令

    int CommandIndex; // 消息序号
    bool SnapshotVaild; // 快照有效
    std::string Snapshot;
    int SnapshotTerm; // 快照属于的任期
    int SnapshotIndex; 

public:
    ApplyMsg()
        :CommandValid(false),
        Command(),
        CommandIndex(-1),
        SnapshotVaild(false),
        Snapshot(),
        SnapshotTerm(-1),
        SnapshotIndex(-1)
        {}
};





#endif 