/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-02 14:25:12
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-02 18:15:03
 * @FilePath: /myRaftKv/src/raftCore/include/raft.h
 * @Description: 
 */
#ifndef SRC_RAFTRPCPROTO_INCLUDE_RAFT_H
#define SRC_RAFTRPCPROTO_INCLUDE_RAFT_H

// raft的具体实现
// 日志是用boost的序列化之后保存的
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/any.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>

#include "persister.h"
#include "config.h"
#include "raftRpcUtil.h"
#include "raftRPC.pb.h"
#include "applyMsg.h"
#include "util.h"

// 网络状态标识，如果出现网络分区设置标志位discinnected, 网络正常就是AppNormal
//  const 变量的初始化可以推迟到运行时进行。 constexpr 变量必须在编译时进行初始化
// const表示只读， constexper表示常量
constexpr int Disconnected = 0; 
constexpr int AppNormal = 1;

/***********************************************************/
/**                     raft投票状态                        **/
/***********************************************************/
constexpr int Killed = 0;   
constexpr int Voted = 1;   // 节点本轮已经投过票了
constexpr int Expire = 2;  // 投票的（消息 or 竞选者）过期了
constexpr int Normal = 3;   // 正常投票状态


/// @brief raft的实现，所有定义的变量都根据笔记里面的实现图
class Raft : public raftRpcProtocol::raftRpc{

public:
    void init(std::vector<std::shared_ptr<RaftRpcUtil>> peers, int me, std::shared_ptr<Persister> persister,
            std::shared_ptr<LockQueue<ApplyMsg>> applychan);

private:
    /*************          辅助变量          ********************/
    std::mutex m_mutex;
    // 协程还未实现
    // std::unique_ptr<monsoon::IOManager> m_ioManager = nullptr;

    /*************          节点状态变量         ********************/
    int m_me; // 自己的节点id
    int m_currentTerm; // 当前的任期
    int m_votedFor; // 如果不是leader节点，需要进行投票，这个记录投票给谁了。

    std::chrono::_V2::system_clock::time_point m_lastResetElectionTime; // 选举超时，这个类可以指定时间，默认从1970-1-1开始
    std::chrono::_V2::system_clock::time_point m_lastResetHearBeatTime; // 心跳超时时间，_V2是预留给未来版本的，其实加不加都一样

    enum Status {FOLLOWER, CANDIDATE, LEADER}; // 三个状态
    Status m_status;


    /*************          log - state相关变量          ******************/
    int m_commitIndex;  // 当前已经提交到日志的id（也就是收到了同步回应的）
    int m_lastApplied; // 已经上交给状态机的最后的log的index

    int m_lastSnapshotIncludeIndex; // 快照中最后一个日志的index
    int m_lastSnapshotIncludeTerm; // 最后一个日志的term

    std::vector<int> m_nextIndex;     // 日志同步进度的数组，里面记录的是已经同步的log
    std::vector<int> m_matchIndex;   // leader已经同步，但是多数follower还没有确认的log的index

    
    std::vector<std::shared_ptr<RaftRpcUtil>> m_peers; // 其他节点的rpc通信
    std::shared_ptr<Persister> m_persister; // 持久化
    std::vector<raftRpcProtocol::LogEntry> m_logs; // 这个是模拟状态机，里面的节点在proto定义了一个三元组<term, index, commond>

    /*************          raft通信相关         ********************/
    std::shared_ptr<LockQueue<ApplyMsg>> m_applyChan; // client从这里拉取日志，模仿go的chan

public:
    /*************          raft间相关         ********************/
    // // leader
    void leaderHeartBeatTicker(); // leader检查是否需要发起心跳，有心跳定时
    void leaderSendSnapShot(int server); // 如果nextIndex在snapshot里面，就要发送snapshot给follower同步
    void leaderUpdateCommitIndex(); // leader更新commitIndex，就是同步的log有大多数follower回应了就更新
    void doHeartBeat();     // leader发起心跳
    bool sendAppendEntries(int server, std::shared_ptr<raftRpcProtocol::AppendEntriesArgs> args, // 发送心跳后，对心跳的回复处理
                         std::shared_ptr<raftRpcProtocol::AppendEntriesReply> reply, std::shared_ptr<int> appendNums);
    void AppendEntries1(const raftRpcProtocol::AppendEntriesArgs* args, raftRpcProtocol::AppendEntriesReply* reply);     // 日志和心跳的rpc发送和接收回应
    // // candidate
    void RequestVote(const raftRpcProtocol::RequestVoteArgs* args, raftRpcProtocol::RequestVoteReply* reply); // 请求投票
    bool sendRequestVote(int server, std::shared_ptr<raftRpcProtocol::RequestVoteArgs> args,
                        std::shared_ptr<raftRpcProtocol::RequestVoteReply> reply, std::shared_ptr<int> votedNum); // 发送请求
    bool UpToDate(int index, int term); // 判断当前节点是否含有最新的日志，选举什么的都能用上
    void doElection();         // 发起选举

    void electionTimeOutTicker(); // 选举超时维护，发现超时就要进行选举，一段时间检查一次，如果有定时器重置了就进睡眠
    int getLastLogIndex(); // 最后一个log的Index
    int getLastLogTerm(); // 最后一个log的Term
    void getLastLogIndexAndTerm(int *lastLogIndex, int *lastLogTerm);
    int getLogTermFromLogIndex(int logIndex);
    bool matchLog(int logIndex, int logTerm); // 对应的index的log是不是匹配的
    void applierTicker();    // 定期向状态机提交日志

    /*************          client通信相关         ********************/
    std::vector<ApplyMsg> getApplyLogs();
    int getNewCommandIndex();
    void getPrevLogInfo(int server, int* preIndex, int* preTerm);
    void GetState(int* term, bool* isLeader); // 获取是不是leader的转态

    void pushMsgToKvServer(ApplyMsg msg); // 写入一个数据到chan(lockQueue)
    void readPersist(std::string data);
    std::string getPerSistData();

    void Start(Op command, int* newLogIndex, int* newLogTerm, bool* isLeader); // 开始通信

    /*************          snapshot - state相关         ********************/
    // // snapshot
    void InstallSnapshot(const raftRpcProtocol::InstallSnapshotRequest *args,
                       raftRpcProtocol::InstallSnapshotResponse *reply);    // 制作快照
    bool CondInstallSnalshot(int lastIncludeTerm, int lastIncludeIndex, std::string snapshot);         // 写快照
            // index表示快照保存到了哪条log， snapshot是旧的快照。
    void Snapshot(int index, std::string snapshot); //把安装到快照里的日志抛弃，然后做新的快照，更新快照的下标。每个节点都主动更新。
    // // state
    int GetRaftStateSize();
    int getSlicesIndexFromLogIndex(int logIndex); // 有了snapshot之后，因为要截断，不能单纯的用logIndex表示在log数组中的位置，这里要找到这个位置

    void persist(); // 持久化



public:
    /***********************************************************/
    /**                     raft rpc                          **/
    /***********************************************************/
    // 三个函数分别是同步日志或心跳， 发送快照， 请求投票
    // 直接继承实现的，内部会调用，这里主要是想内部再直接调用定义的raftRpcUtil里面的三个函数处理数据
    void AppendEntries(google::protobuf::RpcController *controller, const ::raftRpcProtocol::AppendEntriesArgs *request,
                     ::raftRpcProtocol::AppendEntriesReply *response, ::google::protobuf::Closure *done) override;
    void InstallSnapshot(google::protobuf::RpcController *controller,
                        const ::raftRpcProtocol::InstallSnapshotRequest *request,
                        ::raftRpcProtocol::InstallSnapshotResponse *response, ::google::protobuf::Closure *done) override;
    void RequestVote(google::protobuf::RpcController *controller, const ::raftRpcProtocol::RequestVoteArgs *request,
                    ::raftRpcProtocol::RequestVoteReply *response, ::google::protobuf::Closure *done) override;

};




#endif // SRC_RAFTRPCPROTO_INCLUDE_RAFTRPC_H