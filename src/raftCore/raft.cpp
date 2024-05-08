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
/**                   leader发送心跳、日志过程               **/
/***********************************************************/

/// @brief leader发送心跳或者日志事件
void Raft::leaderHeartBeatTicker()
{
    // m_eventLoop.runEvery(1000 * HeartBeatTimeout, std::bind(&Raft::HeartBeatTicker, this));

    while(true)
    {
        while(m_status != LEADER)
        {
            usleep(1000 * HeartBeatTimeout);
        }

        static std::atomic<int32_t> atomicCount;
        atomicCount.store(0);

        // std::chrono::duration<signed long int, std::ratio<1, 1000000000>> suitableSleepTime{};   // 这样写觉得不好，9个0容易写错，用自带的ns就行
        std::chrono::duration<signed long int, std::nano> suitableSleepTime{};      // 设置一个合适的睡眠时间，等待下次心跳触发
        std::chrono::system_clock::time_point wakeTime{};                           // 这个变量要接收now
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            wakeTime = now();
            suitableSleepTime = std::chrono::milliseconds(HeartBeatTimeout) + m_lastResetHearBeatTime - wakeTime; // 睡眠时间（下一次超时时间）
        }

        if(std::chrono::duration<double, std::milli>(suitableSleepTime).count() > 1) // 要睡眠的时间 > 1ms
        {
            std::cout << atomicCount << "\033[1;35m leaderHearBeatTicker();函数设置睡眠时间为: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(suitableSleepTime).count() << " 毫秒\033[0m"
                << std::endl;

            auto start = std::chrono::steady_clock::now();            // 这是时间是从boot启动到现在的时间，是相对的

            usleep(std::chrono::duration_cast<std::chrono::microseconds>(suitableSleepTime).count()); // 睡眠单位 us

            auto end = std::chrono::steady_clock::now();

            std::chrono::duration<double, std::milli> duration = end - start; // 计算睡眠了多少 ms

            // 使用ANSI控制序列将输出颜色修改为紫色
            std::cout << atomicCount << "\033[1;35m leaderHearBeatTicker();函数实际睡眠时间为: " << duration.count()
                << " 毫秒\033[0m" << std::endl;

            ++atomicCount;                  // 计算睡眠次数
        }

        if(std::chrono::duration<double, std::milli>(m_lastResetHearBeatTime - wakeTime).count() > 0)  // 到了下一次心跳的超时
        {
            // 这里是因为如果m_lastResetHearBeatTime没有被重置，wakeTime>m_lastResetHearBeatTime的
            // 如果重置了 m_lastResetHearBeatTime肯定比上面的now()大
            continue;       // 重新心跳计时
        }

        doHeartBeat();      // 心跳超时，发起心跳。
    }
}

/// @brief leader发送心跳，并且同步log或者snapshot。follower收到日志就算心跳
void Raft::doHeartBeat()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_status == LEADER)
    {
        DPrintf("[func-Raft::doHeartBeat()-Leader: {%d}] Leader的心跳定时器触发了且拿到mutex，开始发送AE\n", m_me);

        auto appendNums = std::make_shared<int>(1);

        // 发送日志（可以拆分）
            // 1.1 如果nextIndex(日志同步index) >= lastSnapshotIndex(快照的最后index) 说明日志更新，发送日志
            // 1.2 相反的，说明日志包含了要发送的log，需要发送整个快照，follower接收后丢弃自己的快照，直接用这个最新的快照

        for(int i = 0; i < m_peers.size(); ++i)
        {
            if(i == m_me)
            {
                continue;
            }

            DPrintf("[func-Raft::doHeartBeat()-Leader: {%d}] Leader的心跳定时器触发了 index:{%d}\n", m_me, i);
            myAssert(m_nextIndex[i] >= 1, format("rf.nextIndex[%d] = {%d}", i, m_nextIndex[i]));

            // 1.2 快照包含最新未同步log的情况
            if(m_nextIndex[i] < m_lastSnapshotIncludeIndex)
            {
                std::thread(&Raft::leaderSendSnapShot, this, i).detach();   // 线程发送快照，指定server
                continue;
            }


            // 1.1 log更新的情况，这时候就要找上次同步的log index， 发送[perLogIndex, ]的log
            int prevLogIndex = -1;
            int prevLogTerm = -1;
            getPrevLogInfo(i, &prevLogIndex, &prevLogIndex);

/// TODO 为什么这个要用智能指针包裹？---> 因为protobuf中有嵌套类型，内部就会申请内存，但是要手动释放(delete)
///     使用protobuf的clear()只是把数组初始化为0，不会释放内存，和stl不同
///     使用shared_ptr,析构的时候就会把管理的protobuf内存全部释放，不会有内存泄漏
//  !!!!  这就是上次面试问我的protobuf嵌套有什么问题的答案！！！！
            std::shared_ptr<raftRpcProtocol::AppendEntriesArgs> appendEntriesArgs =         // 发送日志的rpc参数填写
                            std::make_shared<raftRpcProtocol::AppendEntriesArgs>();

            appendEntriesArgs->set_term(m_currentTerm);
            appendEntriesArgs->set_leaderid(m_me);
            appendEntriesArgs->set_prevlogindex(prevLogIndex);
            appendEntriesArgs->set_prevlogterm(prevLogTerm);
            appendEntriesArgs->clear_entries();                 // clear只是把数组置为0，和stl不同
            appendEntriesArgs->set_leadercommit(m_commitIndex);

            // 1.1.1 如果上一条不是已经做成快照的，就要从index一直到最后发送
            // 1.1.2 如果上一条做成快照了，那就只要发送全部的log就行
            if(prevLogIndex != m_lastSnapshotIncludeIndex)
            {
                for(int j = getSlicesIndexFromLogIndex(prevLogIndex) + 1; j < m_logs.size(); ++j)
                {
                    raftRpcProtocol::LogEntry* sendEntryPtr = appendEntriesArgs->add_entries();
                    *sendEntryPtr = m_logs[j];          // 通过指针方式添加，= 是重载的，内部就是cpoy一个m_logs到entries数组中
                }
            }
            else
            {
                for(const auto& log : m_logs)
                {
                    raftRpcProtocol::LogEntry* sendEntryPtr = appendEntriesArgs->add_entries();
                    *sendEntryPtr = log;          // 通过指针方式添加，= 是重载的，内部就是cpoy一个m_logs到entries数组中
                }
            }

            int lastLogIndex = getLastLogIndex();   // 就是看看加入到数组的个数是不是要发送的个数
            // leader对每个节点发送的日志长短不一，但是都保证从prevIndex发送直到最后
            myAssert(appendEntriesArgs->prevlogindex() + appendEntriesArgs->entries_size() == lastLogIndex,
                    format("appendEntriesArgs.PrevLogIndex{%d}+len(appendEntriesArgs.Entries){%d} != lastLogIndex{%d}",
                            appendEntriesArgs->prevlogindex(), appendEntriesArgs->entries_size(), lastLogIndex));


            const std::shared_ptr<raftRpcProtocol::AppendEntriesReply> appendEntriesReply =
                    std::make_shared<raftRpcProtocol::AppendEntriesReply>();

            appendEntriesReply->set_appstate(Disconnected);

            // 3. 发送并且处理返回值，线程做
            std::thread(&Raft::sendAppendEntries, this, i ,appendEntriesArgs, appendEntriesReply, appendNums).detach();
        }

        m_lastResetHearBeatTime = now(); // 对全部的follower发送心跳日志之后就要重置
    }
}

/// @brief
/// @param server
/// @param args
/// @param reply
/// @param appendNums
/// @return 发送是否成功，不是follower是否接收log
bool Raft::sendAppendEntries(int server, std::shared_ptr<raftRpcProtocol::AppendEntriesArgs> args,
                            std::shared_ptr<raftRpcProtocol::AppendEntriesReply> reply, std::shared_ptr<int> appendNums)
{
    // 发送的开头不加锁，论文中要求如果发送log失败应该不断重试retries
    DPrintf("[func-Raft::sendAppendEntries-raft{%d}] leader 向节点{%d}发送AE rpc開始 ， args->entries_size():{%d}", m_me,
          server, args->entries_size());


    bool ok = m_peers[server]->AppendEntries(args.get(), reply.get());

    if (!ok)
    {
        DPrintf("[func-Raft::sendAppendEntries-raft{%d}] leader 向节点{%d}发送AE rpc失敗", m_me, server);
        return ok;
    }
    DPrintf("[func-Raft::sendAppendEntries-raft{%d}] leader 向节点{%d}发送AE rpc成功", m_me, server);
    if (reply->appstate() == Disconnected)          // 节点状态是失败的
    {
        return ok;
    }

    // 这部分涉及到index和term操作，加锁，处理rpc回复
    {
        std::lock_guard<std::mutex> lock(m_mutex);


        // 任期必须相同，不相同的不能更新log
        if(reply->term() > m_currentTerm)
        {
            m_status = FOLLOWER;
            m_votedFor = -1;
            m_currentTerm = reply->term();
            m_lastResetElectionTime = now();
            return ok;
        }
        else if(reply->term() < m_currentTerm)
        {
            DPrintf("[func -sendAppendEntries  rf{%d}]  节点：{%d}的term{%d}<rf{%d}的term{%d}\n", m_me, server, reply->term(),
                m_me, m_currentTerm);
            return ok;
        }

        if(m_status != LEADER)
            return ok;

        // 这里的term是相同的
        myAssert(reply->term() == m_currentTerm,
           format("reply.Term{%d} != rf.currentTerm{%d}   ", reply->term(), m_currentTerm));


        if(!reply->success())                       // 更新失败，说明日志不匹配，前几次同步的log出问题了
        {
            if(reply->updatenextindex() != 100)     // 这是follower失败设定的默认值
            {
                DPrintf("[func -sendAppendEntries  rf{%d}]  返回的日志term相等，但是不匹配，回缩nextIndex[%d]：{%d}\n", m_me,
                        server, reply->updatenextindex());

                m_nextIndex[server] = reply->updatenextindex(); // 重置要同步的index， 不匹配foller会自己要同步的index发回来
            }
        }
        else                                        // 更新成功，日志，任期都匹配了
        {
            *appendNums = *appendNums + 1;          // 同步的follower+1 ，超过半数+1就能够commit
            DPrintf("---------------------------tmp------------------------- 节点{%d}返回true，log同步成功,当前同步节点数量*appendNums{%d}", server,
                *appendNums);

            m_matchIndex[server] = std::max(m_matchIndex[server], args->prevlogindex() + args->entries_size()); // 更新节点已经同步的index
            m_nextIndex[server] = m_matchIndex[server] + 1; // nextIndex设定的最新的都是matchindex+1，但不是多余的，可以判断next - match之间有多少log没有开始同步

            int lastLogIndex = getLastLogIndex();
            myAssert(m_nextIndex[server] <= lastLogIndex + 1,
                    format("error msg:rf.nextIndex[%d] > lastLogIndex+1, len(rf.logs) = %d   lastLogIndex{%d} = %d", server,
                            m_logs.size(), server, lastLogIndex));

            /////// 提交到状态机，前提是满足同步成功回复的数量 > 半数 + 1
/// TODO    是不是可以设计成acks，和kafka一样的三种同步模式

            if(*appendNums >= m_peers.size() / 2 + 1)
            {
                *appendNums = 0;    // 必须设置0，否则会再次提交，没有幂等性

                if(args->entries_size() > 0)    // 更新commitIndex的值，同步了多少log就提交多少
                {
                    DPrintf("args->entries(args->entries_size()-1).logterm(){%d}   m_currentTerm{%d}",
                        args->entries(args->entries_size() - 1).logterm(), m_currentTerm);

/// TODO    没怎么想明白为什么要等有写入才commit https://zhuanlan.zhihu.com/p/517969401
                    // 保证当前任期有日志同步，这段提交的log可能是有不同的term，也就是每个Log可能任期不同
                    // 保证一个特性：领导人完备性（新的leader上任，只有新的leader有了log同步保存，才会让前面的term进行提交）

// 总结：raft的日志有两个性质：（1）如果不同节点的entry的index和term相同，那么这两个entry一定相同
//                          (2)如果不同节点的entry的index和term相同，那么这个evtry之前的log全部相同
// 场景就是：(1)上一个leader同步消息，但是没有commit（问题：可能只有几个follower收到同步）
//        （2）新的leader上任，因为上一任同步的消息没有commit，还有一部分没有同步上一任的log（当前leader日志是最新的，肯定同步到了）
//              (2.1) 这时候如果新leader直接commit了，导致有些folloer没有上一任leader同步的消息，数据不一致
//              (2.2) 如果等到新的leader有数据同步了，!!!!就会从上一任没有commit的地方发起同步（重点！！！），（上一任没有同步到的follower被新的leader同步）
//                           因为新leader肯定有上一任leader发送过来uncommit的日志，所以会把这部分的日志一起发起同步数据一致。
//              (2.3) 如果新任leader直接commit上一任的同步，下次发起同步就不会再带着之前其他follower没有收到的日志同步

//  本质问题就是：不提交上一个leader的uncommit部分。等到下一次一起同步（同步从uncommit的部分开始）（不一定要有新数据，发一个空的entey也行，就是为了把上一任uncommit部分再发一遍）
                    if(args->entries(args->entries_size() - 1).logterm() == m_currentTerm)
                    {
                        DPrintf(
                            "---------------------------tmp------------------------- 当前term有log成功提交，更新leader的m_commitIndex "
                            "from{%d} to{%d}",
                            m_commitIndex, args->prevlogindex() + args->entries_size());
                        // 更新commit就是从上一次uncommit的index开始
                        m_commitIndex = std::max(m_commitIndex, args->prevlogindex() + args->entries_size());
                    }

                    myAssert(m_commitIndex <= lastLogIndex,
               format("[func-sendAppendEntries,rf{%d}] lastLogIndex:%d  rf.commitIndex:%d\n", m_me, lastLogIndex,
                      m_commitIndex));
                }
            }
        }
    }
    return ok;
}

/// @brief 指定server发送leader的快照，在最新log已经被做成snapshot的情况下
/// @param server
void Raft::leaderSendSnapShot(int server)
{
   // 1. 设置发送快照的protobuf参数
    std::unique_lock<std::mutex> lock(m_mutex);

    raftRpcProtocol::InstallSnapshotRequest args;
    args.set_leaderid(m_me);
    args.set_term(m_currentTerm);
    args.set_lastsnapshotincludeindex(m_lastSnapshotIncludeIndex);
    args.set_lastsnapshotincludeterm(m_lastSnapshotIncludeTerm);
    args.set_data(m_persister->ReadSnapshot());     // 全部snapshot发送

    raftRpcProtocol::InstallSnapshotResponse reply;

    lock.unlock();

    bool ok = m_peers[server]->InstallSnapshot(&args, &reply);      // 发送rpc

    lock.lock();

    if(!ok)
    {
        return;     // 发送失败
    }

    if(m_status != LEADER || m_currentTerm != args.term())          // 中间发送不是加锁的，可能当前的状态都变了
    {
        return;
    }

    // 2. 处理回复
    if(reply.term() > m_currentTerm)                                // 对方的任期比leader还高，可能是脑裂过
    {
        m_currentTerm = reply.term();
        m_votedFor = -1;                // 要转变为follower
        m_status = FOLLOWER;

        persist();                      // 有状态变化

        m_lastResetElectionTime = now(); // 变成了follower所有参数都要开始设置
        return;
    }

    m_matchIndex[server] = args.lastsnapshotincludeindex(); // 最后的快照index就是已经同步并且得到确认的
    m_nextIndex[server] = m_matchIndex[server] + 1;         // 下一个要同步的当然是最新同步的 + 1
}

/// @brief folloer处理leader同步的log，函数调用流程raftRpcUtil->AppendEntries()->this->AppendEntries()->this->AppendEntries1()
/// @param controller
/// @param request
/// @param response
/// @param done
void Raft::AppendEntries(google::protobuf::RpcController *controller, const ::raftRpcProtocol::AppendEntriesArgs *request,
                        ::raftRpcProtocol::AppendEntriesReply *response, ::google::protobuf::Closure *done)
{
    AppendEntries1(request, response);
    done->Run();
}

/// @brief 处理log，follower加入leader同步的log
/// @param args
/// @param reply
void Raft::AppendEntries1(const raftRpcProtocol::AppendEntriesArgs *args, raftRpcProtocol::AppendEntriesReply *reply)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    reply->set_appstate(AppNormal);         // 收到消息，证明网络是正常的

    if(args->term() < m_currentTerm)        // 过期leader(可能是网络分区之后刚加上来的，所以term远低于当前term)
    {
        reply->set_success(false);
        reply->set_term(m_currentTerm);
        reply->set_updatenextindex(-100);   // 设置为-100,leader收到之后这里是什么也不做，就当没有收到，下一任leader会重发的

        DPrintf("[func-AppendEntries-rf{%d}] 拒绝了 因为Leader{%d}的term{%v}< rf{%d}.term{%d}\n", m_me, args->leaderid(),
            args->term(), m_me, m_currentTerm);
        return;                             // 注意从过期的领导人收到消息不要重设定时器
    }

    DEFER {
        persist();                          // 放在最后了，DEFER宏一直报错
    };

    if(args->term() > m_currentTerm)        // 任期落后了，就要全部打回follower
    {
        m_status = FOLLOWER;
        m_currentTerm = args->term();
        m_votedFor = -1;
    }
    myAssert(args->term() == m_currentTerm, format("assert {args.Term == rf.currentTerm} fail"));

    m_status = FOLLOWER;                    // 收到心跳，可能当前是候选人，直接就变成FOLLOWER
    m_lastResetElectionTime = now();        // 变成follower肯定要超时设置


    // 更新日志有三种情况
        // 1. 上次更新的日志超过了follower最后同步的日志，出错了。中间还有一部分没有更新到
        // 2. 上次更新的日志比folloer的snapshot还要旧（说明是rpc延迟了）
        // 3. 正常日志，上次更新的和follower的lastLogINdex是一样的

    if(args->prevlogindex() > getLastLogIndex())                // 第一种情况
    {
        reply->set_success(false);
        reply->set_term(m_currentTerm);
        reply->set_updatenextindex(getLastLogIndex() + 1);      // 这里发送follower当前的更新进度+1,+1是请求下一个，leader收到就会更新nextIndex[i]
        return;
    }
    else if(args->prevlogindex() < m_lastSnapshotIncludeIndex)  // 第二种情况
    {
        reply->set_success(false);
        reply->set_term(m_currentTerm);
        reply->set_updatenextindex(m_lastSnapshotIncludeIndex + 1);// 让leader跟上快照
    }


    if(matchLog(args->prevlogindex(), args->prevlogterm()))     // index必须满足上面的条件，term必须相同
    {
        // 即使prevLoginIndex是对的，也不能单纯的直接加入logs，还是要判断每个log是不是符合
        // 1. 每个Log的longindex > follower的lastLogIndex 成立
        // 2. 如果index和term都相同，但是commond不同，就违反了raft的日志同步原则，报错
        // 3. 如果log的任期不同，前面已经判断过leader的term肯定是更大的，进行覆盖（这里就不能保证一致性了）

        for(int i = 0; i < args->entries_size(); ++i)
        {
            auto log = args->entries(i);

            if(log.logindex() > getLastLogIndex()) // 情况1
            {
                m_logs.emplace_back(log);           // log临时变量，不使用了
            }
            else
            {
                if(m_logs[getSlicesIndexFromLogIndex(log.logindex())].logterm() == log.logterm() && // 情况2, 相同位置不同指令
                    m_logs[getSlicesIndexFromLogIndex(log.logindex())].command() != log.command())
                {
                        myAssert(false, format("[func-AppendEntries-rf{%d}] 两节点logIndex{%d}和term{%d}相同，但是其command{%d:%d}   "
                                " {%d:%d}却不同！！\n",
                                m_me, log.logindex(), log.logterm(), m_me,
                                m_logs[getSlicesIndexFromLogIndex(log.logindex())].command(), args->leaderid(),
                                log.command()));
                }

                if(m_logs[getSlicesIndexFromLogIndex(log.logindex())].logterm() != log.logterm()) // 情况3，相同位置不同任期，覆盖（数据不一致了）
                {
                    m_logs[getSlicesIndexFromLogIndex(log.logindex())] = log;
                }
            }
        }

        myAssert(
            getLastLogIndex() >= args->prevlogindex() + args->entries_size(),
            format("[func-AppendEntries1-rf{%d}]rf.getLastLogIndex(){%d} != args.PrevLogIndex{%d}+len(args.Entries){%d}",
                m_me, getLastLogIndex(), args->prevlogindex(), args->entries_size()));

        // 日志更新完成
        reply->set_success(true);
        reply->set_term(m_currentTerm);
        return;
    }
    else  // if(matchLog(args->prevlogindex(), args->prevlogterm()))   term不匹配的问题
    {
        // 任期不同但是index都相同，可能是leader接收完数据就宕机，没同步。立马开始变成了follower，新的leader发送过来，自己index相同但是本身term就是小一个

        reply->set_updatenextindex(args->prevlogindex()); // 要求更新的index就是这个，只是任期不同而已

        // 出现上面那种情况，leader本身收到的数据就是要丢弃的数据了，找到那个term
        // 设置为一次倒退多个匹配更好，否则一个个倒退找不同term的就发送多个rpc了
        for(int index = args->prevlogindex(); index >= m_lastSnapshotIncludeIndex; --index)
        {
            if(getLogTermFromLogIndex(index) != getLogTermFromLogIndex(args->prevlogindex())) // 找到不同的第一个任期index，下次同步就是这个index，会覆盖index之后的
            {
                reply->set_updatenextindex(index + 1);
                break;
            }
        }

        reply->set_success(false);           // 不同任期肯定失败
        reply->set_term(m_currentTerm);
        return;
    }
}

/***********************************************************/
/**                      raft通用函数                       **/
/***********************************************************/

/// @brief 在raft初始化阶段就是detch线程，不断执行，作用是不断和kvserver通信，传输commond
void Raft::applierTicker()
{
    while(true)
    {
        m_mutex.lock();

        if(m_status == LEADER)
        {
            DPrintf("[Leader Raft::applierTicker() - raft{%d}]  m_lastApplied{%d}   m_commitIndex{%d}", m_me, m_lastApplied,
                    m_commitIndex);
        }

        auto applyMsgs = getApplyLogs();

        m_mutex.unlock();

        if(!applyMsgs.empty())
        {
            DPrintf("[func- Raft::applierTicker()-raft{%d}] 向kvserver报告的applyMsgs的长度为：{%d}", m_me, applyMsgs.size());
        }

        for(auto& message : applyMsgs)
        {
            m_applyChan->Push(message);     // 所有未commit的消息加入有锁队列发送
        }

        // 睡眠发送间隔，隔一段时间发送
/// TODO 这些死循环的都可以用epoll + 协程处理的更好
        sleepNMilliseconds(ApplyInterval);
    }
}

/***********************************************************/
/**                      持久化(snapshot)                  **/
/***********************************************************/

/// @brief 使用protobuf对log进行持久化，去掉了boost依赖，既然rpc用了ptotobuf，就统一。
/// @return 序列化的持久化内容
std::string Raft::perSistData()
{
    std::shared_ptr<Snapshot::PersistRaft> persistRaft =
                        std::make_shared<Snapshot::PersistRaft>();
    // 里面用了嵌套数组，要释放内存，用智能指针方便点
    persistRaft->set_currentterm(m_currentTerm);
    persistRaft->set_votedfor(m_votedFor);
    persistRaft->set_lastsnapshotincludeindex(m_lastSnapshotIncludeIndex);
    persistRaft->set_lastsnapshotincludeterm(m_lastSnapshotIncludeTerm);
    for(int i = 0; i < m_logs.size(); ++i)
    {
        persistRaft->mutable_logs()->Add(m_logs[i].SerializeAsString());    // 直接序列化到string加入bytes数组
    }

    std::string data;

    if(!persistRaft->SerializeToString(&data))
    {
        myAssert(false, "在persistData中使用protobuf序列化失败！");
    }

    return data;
}

bool Raft::CondInstallSnapshot(int lastIncludeTerm, int lastIncludeIndex, std::string snapshot)
{
    return true;
}

/// @brief 保存从kv跳表中读取的序列化文件
/// @param index
/// @param snapshot
void Raft::Snapshot(int index, std::string snapshot)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 已经被做到快照里面或者超出已提交的就不能做
    if (m_lastSnapshotIncludeIndex >= index || index > m_commitIndex) {
        DPrintf(
            "[func-Snapshot-rf{%d}] rejects replacing log with snapshotIndex %d as current snapshotIndex %d is larger or "
            "smaller ",
            m_me, index, m_lastSnapshotIncludeIndex);
        return;
    }

    auto lastLogIndex = getLastLogIndex();  //


    int newLastSnapshotIncludeIndex = index;    // 最新快照包含的最后一个index就是现在保存的位置
    int newLastSnapshotIncludeTerm = m_logs[getSlicesIndexFromLogIndex(index)].logterm();

    std::vector<raftRpcProtocol::LogEntry> trunckedLogs;
// todo :这种写法有点笨，待改进，而且有内存泄漏的风险
    for (int i = index + 1; i <= getLastLogIndex(); i++)
    {
        //注意有=，因为要拿到最后一个日志
        trunckedLogs.push_back(m_logs[getSlicesIndexFromLogIndex(i)]);
    }

    m_lastSnapshotIncludeIndex = newLastSnapshotIncludeIndex;
    m_lastSnapshotIncludeTerm = newLastSnapshotIncludeTerm;
    m_logs = trunckedLogs;              // 从lastIndex之后未提交的部分重新拿到logs中

    m_commitIndex = std::max(m_commitIndex, index);
    m_lastApplied = std::max(m_lastApplied, index);

    m_persister->Save(perSistData(), snapshot);

    DPrintf("[SnapShot]Server %d snapshot snapshot index {%d}, term {%d}, loglen {%d}", m_me, index,
            m_lastSnapshotIncludeTerm, m_logs.size());
    myAssert(m_logs.size() + m_lastSnapshotIncludeIndex == lastLogIndex,
            format("len(rf.logs){%d} + rf.lastSnapshotIncludeIndex{%d} != lastLogjInde{%d}", m_logs.size(),
                    m_lastSnapshotIncludeIndex, lastLogIndex));

}

/***********************************************************/
/**                     kvServer通信                      **/
/***********************************************************/

/// @brief 客户端和leader通信，客户端发送commond给raft集群。leader保存到logs（同步follower日志）
/// @param command
/// @param newLogIndex
/// @param newLogTerm
/// @param isLeader
void Raft::Start(Op command, int *newLogIndex, int *newLogTerm, bool *isLeader)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_status != LEADER)                          // 只有leader才能和外界通信！！！！
    {
        DPrintf("[func-Start-rf{%d}]  is not leader");
        *newLogIndex = -1;
        *newLogTerm = -1;
        *isLeader = false;
        return;
    }

    raftRpcProtocol::LogEntry newLogEntry;
    newLogEntry.set_command(command.asString());
    newLogEntry.set_logterm(m_currentTerm);
    newLogEntry.set_logindex(getNewCommandIndex());
    m_logs.emplace_back(newLogEntry);               // 保存到logs，但是没有同步，没有commit

    int lastLogIndex = getLastLogIndex();
/// TODO 这里的做法应该是leader收到新log就应该和follower同步，但是没有做，只是等待心跳触发一起发送日志
    /// 所以应该分离心跳和同步日志功能，这里单独同步日志（PS:单独功能只要发送不同参数就行）
    DPrintf("[func-Start-rf{%d}]  lastLogIndex:%d,command:%s\n", m_me, lastLogIndex, &command);

    persist();
    *newLogIndex = newLogEntry.logindex();
    *newLogTerm = newLogEntry.logterm();
    *isLeader = true;
}

/// @brief 新的客户端commond应该分配的id
/// @return 分配的id为最后的logid + 1
int Raft::getNewCommandIndex()
{
    auto lastLogIndex = getLastLogIndex();
    return lastLogIndex + 1;
}

/// @brief 返回当前的term和是不是leader
/// @param term
/// @param isLeader
void Raft::GetState(int *term, bool *isLeader)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    *term = m_currentTerm;
    *isLeader = (m_status == LEADER);
}

/// @brief 返回没有提交的日志
/// @return 返回从客户端最后加入leader的日志到最后commit的日志，也就是
std::vector<ApplyMsg> Raft::getApplyLogs()
{
    std::vector<ApplyMsg> applyMsgs;
    myAssert(m_commitIndex <= getLastLogIndex(), format("[func-getApplyLogs-rf{%d}] commitIndex{%d} >getLastLogIndex{%d}",
                                                      m_me, m_commitIndex, getLastLogIndex()));

    while(m_lastApplied < m_commitIndex)
    {
        m_lastApplied++;        // 下一个才是真的没有commit要先++
        myAssert(m_logs[getSlicesIndexFromLogIndex(m_lastApplied)].logindex() == m_lastApplied,
             format("rf.logs[rf.getSlicesIndexFromLogIndex(rf.lastApplied)].LogIndex{%d} != rf.lastApplied{%d} ",
                    m_logs[getSlicesIndexFromLogIndex(m_lastApplied)].logindex(), m_lastApplied));

        ApplyMsg applyMsg;
        applyMsg.CommandValid = true;
        applyMsg.SnapshotVaild = false;     // 这段还在logs[]中，就说明没做快照
        applyMsg.Command = m_logs[getSlicesIndexFromLogIndex(m_lastApplied)].command();
        applyMsg.CommandIndex = m_lastApplied;
        applyMsgs.emplace_back(applyMsg);
    }

    return applyMsgs;
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

/// @brief 同上
/// @return 最后一个log的term
int Raft::getLastLogTerm()
{
    int _ = -1;
    int lastLogTerm = -1;

    getLastLogIndexAndTerm(&_, &lastLogTerm);
    return lastLogTerm;
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

/// @brief 持久化数据，具体数据看定义的
void Raft::persist()
{
    auto data = perSistData();
    m_persister->SaveRaftState(data);       // 做法就是把data写入stringstream中
}


/// @brief 找到上次同步的index和term，分为两种情况，具体见函数内部
/// @param server
/// @param preIndex
/// @param preTerm
void Raft::getPrevLogInfo(int server, int *preIndex, int *preTerm)
{
    // 1. 如果下一次要同步的log就是最新的，那么上次同步的log就是已经被snapshot的
    if(m_nextIndex[server] == m_lastSnapshotIncludeIndex + 1)
    {
        *preIndex = m_lastSnapshotIncludeIndex;
        *preTerm = m_lastSnapshotIncludeTerm;
        return;
    }

    // 2. 上次发送的还没有做成snapshot
    auto nextIndex = m_nextIndex[server];
    *preIndex = nextIndex - 1;
    *preTerm = m_logs[getSlicesIndexFromLogIndex(*preIndex)].logterm(); // 这里是因为当前的term不一定是上一条log的任期，所以要去找上一条的term
}


/// @brief 在上一次没有被做成快照的情况下，需要找它的index，因为当前index和logs是不对应的，前面保存了snapshot
/// @param logIndex
/// @return 找到的上一的真实下标
int Raft::getSlicesIndexFromLogIndex(int logIndex)
{
    // log的index必须是在快照之后的，不然就应该是快照的term
    myAssert(logIndex > m_lastSnapshotIncludeIndex,
           format("[func-getSlicesIndexFromLogIndex-rf{%d}]  index{%d} <= rf.lastSnapshotIncludeIndex{%d}", m_me,
                  logIndex, m_lastSnapshotIncludeIndex));

    int lastLogIndex = getLastLogIndex();   // 从m_logs的最后一条拿

    // 如果当前的最后一条logs比要找的小，就会出问题说明不存在
    myAssert(logIndex <= lastLogIndex, format("[func-getSlicesIndexFromLogIndex-rf{%d}]  logIndex{%d} > lastLogIndex{%d}",
                                            m_me, logIndex, lastLogIndex));

    int sliceIndex = logIndex - m_lastSnapshotIncludeIndex - 1;  // 真实的位置就是这个log的index-快照最后保存的位置
    return sliceIndex;
}

/// @brief 比较任期相同、logIndex不能超过当前快照index，必须大于当前最小的logIndex
/// @param logIndex
/// @param logTerm
/// @return 当前server的任期与传入的是否相同
bool Raft::matchLog(int logIndex, int logTerm)
{
    myAssert(logIndex >= m_lastSnapshotIncludeIndex && logIndex <= getLastLogIndex(),
           format("不满足：logIndex{%d}>=rf.lastSnapshotIncludeIndex{%d}&&logIndex{%d}<=rf.getLastLogIndex{%d}",
                  logIndex, m_lastSnapshotIncludeIndex, logIndex, getLastLogIndex()));
    return logTerm == getLogTermFromLogIndex(logIndex);
}


/// @brief 通过index找到term，通过log拿出来
/// @param logIndex
/// @return 该index在logs中找到的term
int Raft::getLogTermFromLogIndex(int logIndex)
{
    // 首先保证longIndex是对的，不是在快照里面的
    myAssert(logIndex >= m_lastSnapshotIncludeIndex,
           format("[func-getSlicesIndexFromLogIndex-rf{%d}]  index{%d} < rf.lastSnapshotIncludeIndex{%d}", m_me,
                  logIndex, m_lastSnapshotIncludeIndex));

    // 保证logIndex不超出当前最大的logsindex
    int lastLogIndex = getLastLogIndex();
    myAssert(logIndex <= lastLogIndex, format("[func-getSlicesIndexFromLogIndex-rf{%d}]  logIndex{%d} > lastLogIndex{%d}",
                                            m_me, logIndex, lastLogIndex));

    if(logIndex == m_lastSnapshotIncludeIndex)
    {
        return m_lastSnapshotIncludeTerm;
    }


    return m_logs[getSlicesIndexFromLogIndex(logIndex)].logterm();
}

/// @brief 获取raft的状态机（日志logs的大小）
/// @return logs的长度
int Raft::GetRaftStateSize()
{
    return m_persister->GetRaftStateSize();
}