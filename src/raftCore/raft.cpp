#include "raft.h"

#include <muduo/net/EventLoop.h>
#include "config.h"

/// @brief 初始化一个raft服务的所有内容
/// @param peers     所有节点的rpc通信 
/// @param me        标识自己，不给自己通信
/// @param persister 持久化类，每个节点都要自动持久化
/// @param applychan 和kvServer通信的chan，这里用queue实现异步chan
void Raft::init(std::vector<std::shared_ptr<RaftRpcUtil>> peers, int me, std::shared_ptr<Persister> persister, 
                std::shared_ptr<LockQueue<ApplyMsg>> applychan)
{
    m_peers = peers;                // 所有节点通信类
    m_persister = persister;        // 持久化
    m_me = me;

    {     // 需要上锁的区块，防止其他线程抢夺导致未初始化
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_applyChan = applychan;
        m_currentTerm = 0;          // 初始任期为0
        m_status = FOLLOWER;        // 一开始所有server都是follower，有一个超时之后才进行选举leader
        m_commitIndex = 0;
        m_lastApplied = 0;

        m_logs.clear();
        m_nextIndex.resize(peers.size());  // 初始化日志同步进度的大小
        m_matchIndex.resize(peers.size()); // 发送同步没确认的消息，最多就是server的数量
        m_votedFor = -1;            // 初始状态没有给人投票

        m_lastSnapshotIncludeIndex = 0;
        m_lastSnapshotIncludeTerm = 0;
        m_lastResetElectionTime = now(); // 当前时间设置一个充值点，now()用的是choron
        m_lastResetHearBeatTime = now();

        readPersist(m_persister->ReadRaftState()); // 读取raft之前崩溃的状态，没崩溃就是初始化状态不变
        if(m_lastSnapshotIncludeIndex > 0) // 这里说明了有数据从崩溃中恢复
        {
            m_lastApplied = m_lastSnapshotIncludeIndex; // 最后的leader未同步的index就是快照的最后一个
            // TODO 崩溃恢复不能读取commitIndex，因为快照保存的是已同步已提交的，lastAplied和CommitIndex中间还有一段
            //      如果同步了m_commitIndex，中间那段就会重复同步，没有幂等性
        }
        DPrintf("[Init&ReInit] Sever %d, term %d, lastSnapshotIncludeIndex {%d} , lastSnapshotIncludeTerm {%d}", m_me,
          m_currentTerm, m_lastSnapshotIncludeIndex, m_lastSnapshotIncludeTerm);
    }

    // 协程相关，未实现
    // m_ioManager = std::make_unique<monsoon::IOManager>(FIBER_THREAD_NUM, FIBER_USE_CALLER_THREAD); 

    {
        // 未实现的部分，先使用线程代替协程
        // 启动三个线程，分别是 超时选举定时器，心跳定时器，提交日志定时器（定时提交到状态机）
        std::thread t1(&Raft::leaderHeartBeatTicker, this);
        t1.detach();

        std::thread t2(&Raft::electionTimeOutTicker, this);
        t2.detach();

        std::thread t3(&Raft::applierTicker, this);
        t3.detach();
    }
}


/***********************************************************/
/**                     leader选举过程                      **/
/*electionTimeOutTicker()-> doElection()->sendRequestVote()->RequestVote()*/
/***********************************************************/
/// @brief 超时选举。
void Raft::electionTimeOutTicker()
{
    while(true)
    {
        // 是leader不能进行选举，所以要睡眠等待
        while(m_status == LEADER)
        {
            // usleep不会完全让出cpu，要是协程不让出就直接阻塞了
            usleep(HeartBeatTimeout); // 超时选举时间一般比心跳时间长，所以每次心跳唤醒看看
        }

        // 1. 随机计算下一次超时时间，不是不变的
        // 这是时间间隔， 精度为1000000000 us
        std::chrono::duration<signed long int, std::ratio<1, 1000000000>> suitableSleepTime{};
        std::chrono::system_clock::time_point wakeTime{};  // 就是当前时钟的时间点

        {
            std::lock_guard<std::mutex> lock(std::mutex);
            wakeTime = now();
            // 随机生成超时时间, 这个时间就是下一次的超时时间
            // 最后一次选举时间 + 随机时间 - now() = 下一次选举时间的间隔，使用间隔睡眠
            suitableSleepTime = getRandomizeElectionTimeout() + m_lastResetElectionTime - wakeTime;
        }

        // 2. 睡眠到下一次超时时间
        // 判断下一次的超时间间隔有没有超过1ms，如果超过了就要睡眠等待让出cpu
        // 睡眠结束之后 判断期间有没有定时器重置，如果有，continue， 如果没有，执行选举
        if(std::chrono::duration<double, std::milli>(suitableSleepTime).count() > 1)
        {
            auto start = std::chrono::steady_clock::now();
            // 睡眠这么久，单位是us
            usleep(std::chrono::duration_cast<std::chrono::microseconds>(suitableSleepTime).count());

            auto end = std::chrono::steady_clock::now();

            // 计算时间差并输出结果（单位为毫秒）
            std::chrono::duration<double, std::milli> duration = end - start;

            // 使用ANSI控制序列将输出颜色修改为紫色
            std::cout << "\033[1;35m electionTimeOutTicker();函数设置睡眠时间为: "
                        << std::chrono::duration_cast<std::chrono::milliseconds>(suitableSleepTime).count() << " 毫秒\033[0m"
                        << std::endl;
            std::cout << "\033[1;35m electionTimeOutTicker();函数实际睡眠时间为: " << duration.count() << " 毫秒\033[0m"
                        << std::endl;
        }
        
        // 3. 判断期间有没有重置超时时间
        // m_lastResetElectionTime在收到心跳或者收到entryj就会重置
        // 如果期间重启了选举超时时间m_lastResetElectionTime就会变大，他与上一次记录的时间间隔超过1ms就说明重置了选举时间
        // 那就不能进行选举
        if(std::chrono::duration<double, std::milli>(m_lastResetElectionTime - wakeTime).count() > 0)
        {
            continue;
        }
        
        // 超时了，进行选举
        doElection();
    }
    // std::function<void()> timerCallBack = [](){

    // };
    // // 添加定时事件到epoll，定时调用
    // eventLoop.runEvery(HeartBeatTimeout, timerCallBack);
}


/// @brief 超时进行选举1. 换状态，2.发送选举请求rpc
void Raft::doElection()
{
    std::lock_guard<std::mutex> lock(m_mutex); // 期间需要上锁，防止其他candidate发送投票，而这个cnadidate给别人投票了

    if(m_status == LEADER)  // leader不能选举
        return;
    
    DPrintf("[       ticker-func-rf(%d)              ]  选举定时器到期且不是leader，开始选举 \n", m_me);

    m_status = CANDIDATE;    // 改变状态

/// TODO  这样做是不是在脑裂的时候会出现问题？？？？？？
    m_currentTerm += 1;      // 选举新leader任期+1
    m_votedFor = m_me;       // 每个candidate先给自己投一票，也防止zaigei 其他人投票

    persist();               // 任何log变化的时候都应该持久化一次（// TODO 应该考虑下io耗时的问题）

    std::shared_ptr<int> votedCnt = std::make_shared<int>(1); // 1票是自己投的，投票计数
    m_lastResetElectionTime = now(); // 设置下最后超时时间，防止再次判断调用选举函数

    //  开始发送requestVote rpc请求, 除自己之外的所有server
    for(int i = 0; i < m_peers.size(); ++i)
    {
        if(i == m_me)
            continue;
        
        int lastLogIndex = -1;
        int lastLogTerm = -1;
        getLastLogIndexAndTerm(&lastLogIndex, &lastLogTerm); // 获取自己的Log和term的下标，因为follower投票要投给log最新，term最大的

        std::shared_ptr<raftRpcProtocol::RequestVoteArgs> requestVoreArgs =  // 设置rpc参数
            std::make_shared<raftRpcProtocol::RequestVoteArgs>();
        
        requestVoreArgs->set_term(m_currentTerm);
        requestVoreArgs->set_candidateid(m_me);
        requestVoreArgs->set_lastlogindx(lastLogIndex);
        requestVoreArgs->set_lastlogterm(lastLogTerm);

        auto requestVoteReply = std::make_shared<raftRpcProtocol::RequestVoteReply>(); // 请求回复

        // 创建线程发送
        std::thread(&Raft::sendRequestVote, this, i, requestVoreArgs, requestVoteReply, votedCnt).detach();
    }
}

/// @brief 这个函数由doElection()设置了请求参数，利用rpc发送
/// @param server 发送到哪个节点
/// @param args
/// @param reply
/// @param votedNum
/// @return rpc是否发送成功，而不是被投票是否成功
bool Raft::sendRequestVote(int server, std::shared_ptr<raftRpcProtocol::RequestVoteArgs> args, 
                    std::shared_ptr<raftRpcProtocol::RequestVoteReply> reply, std::shared_ptr<int> votedNum)
{
    // 1. 先发送，保证发送不保证送达
    auto start = now();
    DPrintf("[func-sendRequestVote rf{%d}] 向server{%d} 发送 RequestVote 开始", m_me, m_currentTerm, getLastLogIndex());
    bool ok = m_peers[server]->RequestVote(args.get(), reply.get());   // 调用raftRpcUtil的requestVote函数进行rpc通信
    DPrintf("[func-sendRequestVote rf{%d}] 向server{%d} 发送 RequestVote 完毕，耗时:{%d} ms", m_me, m_currentTerm,
          getLastLogIndex(), now() - start);

    if(!ok)
        return ok;
    
    // 2. 进行回应处理
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(reply->term() > m_currentTerm)  // 对方的term比自己的要大，立即变回follower
        {
            m_status = FOLLOWER;
            m_currentTerm = reply->term(); // 跟随任期
            m_votedFor = -1;                // 现在是follower，可以给其他人投票

            persist();      // 这里持久化了上面更改的内容，就应该持久化

            return true;  // 收到回复了，说明rpc发送成功了
        }
        else if(reply->term() < m_commitIndex)  // 任期小于自己，就给自己投票
        {
            return true;
        }

        myAssert(reply->term() == m_currentTerm, format("assert {reply.Term==rf.currentTerm} fail")); // 相同任期打印信息不断言

        // 否则就是term相同的，要比较lastLogIndex
        if(!reply->votegranted())  // 没有给自己投票，可能是因为给别人投了（别人先发）
        {
            return true;
        }

        *votedNum = *votedNum + 1; // 上面判断没有发生没给自己投票的情况，所以是给自己投了
        if(*votedNum >= m_peers.size() / 2 + 1) // 超过半数 + 1，自己肯定当leader了，其他人不可能有比自己多的票
        {
            *votedNum = 0;
            if(m_status == LEADER) // 一个人当两次领导，不正常
            {
                // false断言退出，打印信息
                myAssert(false, format("[func-sendRequestVote-rf{%d}]  term:{%d} 同一个term当两次领导，error", m_me, m_currentTerm));
            }

            // 变leader
            m_status = LEADER;
            DPrintf("[func-sendRequestVote rf{%d}] elect success  ,current term:{%d} ,lastLogIndex:{%d}\n", m_me, m_currentTerm,
                    getLastLogIndex());
                
            
            int lastLogIndex = getLastLogIndex();
            for(int i = 0; i < m_nextIndex.size(); ++i)
            {
                m_nextIndex[i] = lastLogIndex + 1; // 待确认的消息，这里的意思就是从i开始其他follwer待确认的消息就是这个新的leader的最新日志index
                m_matchIndex[i] = 0;               // 换领导了，日志匹配就是新的
            }

            std::thread(&Raft::doHeartBeat, this).detach(); // 发起心跳，里面也要通知其他节点，自己是leader了

            persist(); // 上面的相关信息都变化了，要持久化
        }
        return true;
    }

}

/// @brief 直接调用RequestVote函数处理投票，上面远程调用利用stub发送，这里就是服务端的远程调用处理函数
/// @param controller 
/// @param request 
/// @param response 
/// @param done 
void Raft::RequestVote(google::protobuf::RpcController *controller, const ::raftRpcProtocol::RequestVoteArgs *request, 
                    ::raftRpcProtocol::RequestVoteReply *response, ::google::protobuf::Closure *done)
{
    // 进行处理
    RequestVote(request, response);
    done->Run();
}

/// @brief 处理投票过程
/// @param args 
/// @param reply 
void Raft::RequestVote(const raftRpcProtocol::RequestVoteArgs *args, raftRpcProtocol::RequestVoteReply *reply)
{
    std::lock_guard<std::mutex> lock(m_mutex);  // 选举过程不能被干涉，万一两个候选人同时投票就出错了

    DEFER {                                 // 最后follower本身需要持久化，因为状态变化了
        persist();
    };

    // 1. 处理term
        // 1.1 term < self term 不能投票
        // 1.2 term > self term 还要判断日志是不是更新
        // 1.3 term == self term 要对比谁的日志新，lastLogIndex
    
    if(args->term() < m_currentTerm)
    {
        reply->set_term(m_currentTerm);     // 返回当前任期，接收的canadiate立马变成follower
        reply->set_votegranted(false);      // 不给他投票
        reply->set_votestate(Expire);       // 设置竞选者失效，也就是他不再是竞选者
        return;
    }

    if(args->term() > m_currentTerm)
    {
        m_status = FOLLOWER;                // 因为candidate发送投票请求是给所有人发送，其他candidate发生任期比其他的小就自动降为follower
        m_currentTerm = args->term();       // 跟随更大的任期
        m_votedFor = -1;                    // 现在是变成了follower的状态，所以设置为可以投票
    }

    myAssert(args->term() == m_currentTerm, format("[func--rf{%d}] 前面校验过args.Term==rf.currentTerm，这里却不等", m_me));
    if(args->term() == m_currentTerm)       // 这里其实可以不用判断，为了条件好看，但也不会出错
    {
        // 2. 相同的term判断日志index
            // 2.1 logIndex > self log  
                // 2.1.1 如果已经投过票了, 不投
                // 2.1.2 如果没有投过，投票
            // 2.2 logIndex < self log   不投
        
        int lastLogTerm = getLastLogTerm();
        if(!UpToDate(args->lastlogindx(), args->lastlogterm()))     // 只有candidate的日志新的程度 >= 接受者的日志新的程度才能投票（新的程度用log index和log term判断）
        {
            reply->set_term(m_currentTerm); // 这里面是判断的不满足的条件，注意if(!)里面是取反
            reply->set_votestate(Voted);    // 日志比对方新，就设置为投过票了，有更新的candidate来过
            reply->set_votegranted(false);

            return;
        }

/// TODO 不是应该if(m_votedFor != -1 || m_votedFor == args->candidateid()) ????
///     出现投过票的原因是，如果reply出现网络问题，对方没收到回复，就会重发请求。所以会出现重复投票
        if(m_votedFor != -1 && m_votedFor != args->candidateid()) // 1.已经投过票了(先到先得) 2.已经投过票给当前的canidateid了
        {
            reply->set_term(m_currentTerm);
            reply->set_votestate(Voted);
            reply->set_votegranted(false);
            return;
        } 
        else                                                     // 没有投过票 或者已经给当前candidate投过票了
        {
            m_votedFor = args->candidateid();// 设置投票给谁了
            m_lastResetElectionTime = now(); // 这里已经选举了，所以认为是出leader了，当前follower本身就要开始超时定时，等待新leader发送心跳

            reply->set_term(m_currentTerm);
            reply->set_votestate(Normal);   // 正常投票
            reply->set_votegranted(true);
            return;
        }
    }

}

/***********************************************************/
/**                     辅助函数                     **/
/***********************************************************/

/// @brief 根据日志是否空填写最后一个log的index和term
/// @param lastLogIndex 
/// @param lastLogTerm 
void Raft::getLastLogIndexAndTerm(int *lastLogIndex, int *lastLogTerm)
{
    
    if(m_logs.empty())   // 如果日志是空的，可能已经做成快照了，就选择快照的最后一个
    {
        *lastLogIndex = m_lastSnapshotIncludeIndex;
        *lastLogTerm = m_lastSnapshotIncludeTerm;
    }
    else            
    {
        *lastLogIndex = m_logs.rbegin()->logindex();
        *lastLogTerm = m_logs.rbegin()->logterm();
    }
}

/// @brief 分为logs空和非空
/// @return 最后一个log的index
int Raft::getLastLogIndex()
{
    int lastLogIndex = -1;
    int _ = -1;

    getLastLogIndexAndTerm(&lastLogIndex, &_);
    return lastLogIndex;
}

/// @brief 判断传入的log index和log term相比自己的是不是更新
/// @param index 
/// @param term 
/// @return 对方任期更大 or 相同任期，对方long index更大
bool Raft::UpToDate(int index, int term)
{
    int selfLastLogIndex = -1;
    int selfLastLogTerm = -1;
    getLastLogIndexAndTerm(&selfLastLogIndex, &selfLastLogTerm);
    // 1. 任期更大的直接投票
    // 2. 任期相同，但是index更大的也能投票
    return term > selfLastLogTerm || (term == selfLastLogTerm && index >= selfLastLogIndex);
}