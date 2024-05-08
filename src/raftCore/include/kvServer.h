/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-04 20:50:05
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-08 20:26:58
 * @FilePath: /myRaftKv/src/raftCore/include/kvServer.h
 * @Description:
 */
#ifndef SRC_RAFTRPCPROTO_INCLUDE_KVSERVER_H
#define SRC_RAFTRPCPROTO_INCLUDE_KVSERVER_H

#include <iostream>
#include <mutex>
#include <unordered_map>

#include "kvServerRpc.pb.h"
#include "raft.h"
#include "skipList.h"

// 作为服务端要继承，然后重载服务函数
class KvServer : public raftKVRpcProctoc::KvServerRpc{
public:
    KvServer() = delete;
    KvServer(int me, int maxRaftState, std::string nodeInforFileName, short port);

    void StartKVServer();                               // 启动服务
    void DprintfKVDB();                                 //
    void ExecuteAppendOpOnKVDB(Op op);                  // 添加序列化之后的kv到waitApplych
    void ExecuteGetOpOnKVDB(Op op, std::string& value, bool& exist);
    void ExecutePutOpOnKVDB(Op op);

    void Get(const raftKVRpcProctoc::GetArgs* args, raftKVRpcProctoc::GetReply* reply);// 内部调用Get的远程调用

    void GetCommandFromRaft(ApplyMsg message);          // 不是get命令执行，是

    bool IfRequestDuplicate(std::string clientId, int requestId);       // 判断是不是重复的请求命令，通过clientId个消息ID判断，都相同的就是重复请求

    void PutAppend(const raftKVRpcProctoc::PutAppendArgs* args, raftKVRpcProctoc::PutAppendReply* reply); // put命令执行

    void ReadRaftApplyCommandLoop();                    // 一直循环等待raft传来的applyCh

    void ReadSnapshotToInstall(std::string snapshot);   //

    bool SendMessageToWaitChan(const Op &op, int raftIndex);

    void IfNeedToSendSnapShotCommand(int raftIndex, int proportion); // 是否需要制作快照，向raft通知

    void GetSnapShotFromRaft(ApplyMsg message);         // 处理raft发来的 applyChan的内容

    std::string MakeSnapshot();                         // 被调用实际做成快照

public:
    // 这些都是要重载的rpc server函数
    void PutAppend(google::protobuf::RpcController *controller, const ::raftKVRpcProctoc::PutAppendArgs *request,
                 ::raftKVRpcProctoc::PutAppendReply *response, ::google::protobuf::Closure *done) override;

    void Get(google::protobuf::RpcController *controller, const ::raftKVRpcProctoc::GetArgs *request,
            ::raftKVRpcProctoc::GetReply *response, ::google::protobuf::Closure *done) override;


private:
    std::mutex m_mutex;
    int m_me;
    std::shared_ptr<Raft> m_raftNode;
    std::shared_ptr<LockQueue<ApplyMsg>> applyChan;     // 通信管道，线程安全的队列模仿
    int m_maxRaftState;                                 // raft的日志文件最大值，超过就做成snapshot

    std::string m_serializedKVData;                     // 序列化后的kv数据，传输
    SkipList<std::string, std::string> m_skipList;      // kvserver使用跳表做存储
    std::unordered_map<std::string, std::string> m_kvDB;// 内存的一个map存储

    std::unordered_map<int, LockQueue<Op>*> waitApplyCh;// 等待发送的，一个客户端有一个通信chan，int对应id

    std::unordered_map<std::string, int> m_lastRequestId;// 每个client都有多个请求，string是cilentid,int是requestId

    int m_lastSnapShotRaftLogIndex;                     // 最后的Log index

/////////////////serialiazation start ///////////////////////////////
private:

    std::string getSnapshotData()
    {
        std::string serializedKVData = m_skipList.dumpFile();
        return serializedKVData;
    }

    void parseFromString(const std::string& str)
    {
        m_skipList.loadFile(str);
    }
};

#endif