/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-04 20:50:15
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-12 15:12:33
 * @FilePath: /myRaftKv/src/raftCore/kvServer.cpp
 * @Description:
 */
#include "kvServer.h"
#include "rpcConfig.h"
#include "rpcProvider.h"


/// @brief
/// @param me               本raft Node的编号
/// @param maxRaftState     最大的state大小，就是logs的大小
/// @param nodeInforFileName 所有node节点的ip:port信息
/// @param port               当前node的port
KvServer::KvServer(int me, int maxRaftState, std::string nodeInforFileName, short port)
    : m_me(me), m_maxRaftState(maxRaftState), m_skipList(6) // 写死跳表的层数
{
    std::shared_ptr<Persister> persister = std::make_shared<Persister>(me); // 当前节点的持久化类

    applyChan = std::make_shared<LockQueue<ApplyMsg>>();

    m_raftNode = std::make_shared<Raft>();                  // 这就是一个raft的Node



    ////// 整个rpc提供者要接受 客户端的rpc注册，raft直接的客户端注册
    std::thread([this, port]()-> void {
        RpcProvider provider;
        provider.NotifyService(this);       // this是继承了google::protouf::server的子类的，所以直接传就行
        provider.NotifyService(this->m_raftNode.get()); // raft直接通信也要注册，也就是注册本节点的raftNode，也是继承了子类的

        provider.Run(m_me, port);           // 启动当前raftNode的节点。muduo的框架启动
    }).detach();

/// TODO 这里都是用sleep等待的，不太明智，看看怎么改
    ////开启rpc远程调用能力，需要注意必须要保证所有节点都开启rpc接受功能之后才能开启rpc远程调用能力
    ////这里使用睡眠来保证
    std::cout << "raftServer node:" << m_me << " start to sleep to wait all ohter raftnode start!!!!" << std::endl;
    sleep(6);
    std::cout << "raftServer node:" << m_me << " wake up!!!! start to connect other raftnode" << std::endl;


    ///// 配置rpc节点通信的rpc，读取node文件加入除自己之外的节点
    MpRpcConfig config;
    config.LoadConfigFile(nodeInforFileName.c_str());

    std::vector<std::pair<std::string, short>> ipPortVt;

    for(int i = 0; i < INT_MAX - 1; ++i)
    {
        std::string node = "node" + std::to_string(i);

        std::string nodeIp = config.Load(node + "ip");
        std::string nodePortStr = config.Load(node + "port");
        if(nodeIp.empty())
        {
            break;
        }

        ipPortVt.emplace_back(nodeIp, atoi(nodePortStr.c_str()));
    }

    std::vector<std::shared_ptr<RaftRpcUtil>> servers;          // 建立通信连接

    for(int i = 0; i < ipPortVt.size(); ++i)
    {
        if(i == m_me)       // 自己不和自己通信
        {
            servers.push_back(nullptr);
            continue;
        }

        std::string otherNodeIp = ipPortVt[i].first;
        short otherNodePort =  ipPortVt[i].second;

        auto* rpc = new RaftRpcUtil(otherNodeIp, otherNodePort);
        servers.push_back(std::shared_ptr<RaftRpcUtil>(rpc));

        std::cout << "node" << m_me << " 连接node" << i << "success!" << std::endl;
    }

    sleep(ipPortVt.size() - me);    // 一个等一秒，全部node都和其他节点通信连接完成
    // kv的server直接与raft通信，但kv不直接与raft通信，所以需要把ApplyMsg的chan传递下去用于通信，两者的persist也是共用的
    m_raftNode->init(servers, m_me, persister, applyChan);


    m_lastSnapShotRaftLogIndex = 0;
    auto snapshot = persister->ReadSnapshot();
    if(!snapshot.empty())
    {
        ReadSnapshotToInstall(snapshot);
    }

    std::thread(&KvServer::ReadRaftApplyCommandLoop, this).join();
}

/// @brief debug模式下，打印跳表的数据，直接调用的display
void KvServer::DprintfKVDB()
{
    if(!Debug)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);  // 打印不能变化
    m_skipList.displayList();
}

/// @brief 添加key:value到跳表中
/// @param op
void KvServer::ExecuteAppendOpOnKVDB(Op op)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_skipList.insertSetElement(op.m_message.Key, op.m_message.Value);      // 如果存在就更新数值

        m_lastRequestId[op.m_message.ClientId] = op.m_message.RequestId;        // 某个客户端的最后请求id设置，应该是递增的，判断线性一致性
    }

    DprintfKVDB();
}
/// @brief
/// @param op
/// @param value
/// @param exist
void KvServer::ExecuteGetOpOnKVDB(Op op, std::string &value, bool &exist)
{
/// TODO 感觉应该上读写锁，这样不能高并发读了
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        value = "";
        exist = false;

        if(m_skipList.searchElement(op.m_message.Key, value))
        {
            exist = true;
        }
        m_lastRequestId[op.m_message.ClientId] = op.m_message.RequestId;
    }

    DprintfKVDB();
}

/// @brief 插入新的kv，如果存在就更新值
/// @param op
void KvServer::ExecutePutOpOnKVDB(Op op)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_skipList.insertSetElement(op.m_message.Key, op.m_message.Value);
        m_lastRequestId[op.m_message.ClientId] = op.m_message.RequestId;
    }
    DprintfKVDB();
}

/***********************************************************/
/**                      Get                               **/
/***********************************************************/

/// @brief rpc远程调用的get函数，内部调用自定义的get过程函数
/// @param controller
/// @param request
/// @param response
/// @param done
void KvServer::Get(google::protobuf::RpcController *controller, const ::raftKVRpcProctoc::GetArgs *request,
            ::raftKVRpcProctoc::GetReply *response, ::google::protobuf::Closure *done)
{
    this->Get(request, response);
    done->Run();
}

/// @brief kv服务处理客户端的get请求，get请求需要保存到raft的logs中
/// @param args
/// @param reply
void KvServer::Get(const raftKVRpcProctoc::GetArgs *args, raftKVRpcProctoc::GetReply *reply)
{ 
    Op op;                                              // 这个op是请求的op，只有key
    op.m_message.Operation = "Get";
    op.m_message.Key = args->key();
    op.m_message.Value = "";
    op.m_message.ClientId = args->clientid();
    op.m_message.RequestId = args->requestid();         // 请求ID是为了保存线性一致性，永远递增

    int raftIndex = -1;
    int _ = -1;
    bool isLeader = false;
    m_raftNode->Start(op, &raftIndex, &_, &isLeader);   // 保存在raft的logs中，读事件也要保存

    if(!isLeader)                                       // 非leader不能和客户端通信
    {
        reply->set_err(ErrWrongLeader);
        return;
    }

    LockQueue<Op>* chForRaftIndex;                      // 这里加锁，下面不加锁，可能不是同一时间执行的，所以要用一个chan保留
    {                                                   // 插入到waitApplyCh中，拿出这个队列一次性加入多个entry
        std::lock_guard<std::mutex> lock(m_mutex);

        if(waitApplyCh.find(raftIndex) == waitApplyCh.end())    // waitApplyCh就是客户端要执行的命令，如果执行完了就删除
        {
            waitApplyCh.insert({raftIndex, new LockQueue<Op>()});
        }

        chForRaftIndex = waitApplyCh[raftIndex];
    }

    Op raftCommitOp;

    if(!chForRaftIndex->TimeOutPop(CONSENSUS_TIMEOUT, &raftCommitOp))   // 设置执行一个命令的超时时间，这里是如果超时就返回false执行里面的流程
    {
        int _ = -1;
        bool isLeader = false;
        m_raftNode->GetState(&_, &isLeader);

        // 是否重复请求，重复请求就直接返回ok，不重复再发送。
        // 如果是超时的，重复请求get还是对的，可以 拿到
        // get可以重复请求的
        if(IfRequestDuplicate(op.m_message.ClientId, op.m_message.RequestId) && isLeader)
        {
            std::string value;
            bool exist = false;
            ExecuteGetOpOnKVDB(op, value, exist);
            if(exist)
            {
                reply->set_err(OK);
                reply->set_value(value);
            }
            else                                        // 数据库中没有就报错
            {
                reply->set_err(ErrNoKey);
                reply->set_value(value);
            }
        }
        else
        {
            reply->set_err(ErrWrongLeader);             // 换个节点重试，并不是真正的不是leader（找不到重复请求）
        }
    }
    else                // 没有超时，raft执行了添加logs的操作，要给客户端做一个返回（在跳表存储的）
    {
        if(raftCommitOp.m_message.ClientId == op.m_message.ClientId &&          // 请求的get要和拿出的一致
                raftCommitOp.m_message.RequestId == op.m_message.RequestId)
        {
            std::string value;
            bool exist = false;
            ExecuteGetOpOnKVDB(op, value, exist);               // 拿出数据进行设置返回

            if(exist)
            {
                reply->set_err(OK);
                reply->set_value(value);
            }
            else
            {
                reply->set_err(ErrNoKey);
                reply->set_value("");
            }
        }
        else
        {
            reply->set_err(ErrWrongLeader);
        }
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto tmp = waitApplyCh[raftIndex];                          // raftIndex就是要加入raft的logs的位置下标
    waitApplyCh.erase(raftIndex);
    delete tmp;
}



/// @brief （幂等性）是否重复请求，利用clientId和requestId的唯一性判断
/// @param clientId
/// @param requestId
/// @return 是否重复请求
bool KvServer::IfRequestDuplicate(std::string clientId, int requestId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_lastRequestId.find(clientId) == m_lastRequestId.end()) // 没找到相同的就是不重复
    {
        return false;
    }

    return requestId <= m_lastRequestId[clientId];              // 请求id小于就是重复了，因为requesIdyi一直是递增了，为了线性一致性
}

/***********************************************************/
/**                      Put                               **/
/***********************************************************/


/// @brief rpc远程调用的put过程，内部调用自定义的处理过程函数
/// @param controller
/// @param request
/// @param response
/// @param done
void KvServer::PutAppend(google::protobuf::RpcController *controller, const ::raftKVRpcProctoc::PutAppendArgs *request,
                ::raftKVRpcProctoc::PutAppendReply *response, ::google::protobuf::Closure *done)
{
    this->PutAppend(request, response);
    done->Run();
}

/// @brief put到数据库的操作，和get不同的是要判断幂等性，get可以重复操作
/// @param args
/// @param reply
void KvServer::PutAppend(const raftKVRpcProctoc::PutAppendArgs *args, raftKVRpcProctoc::PutAppendReply *reply)
{
    Op op;
    op.m_message.Operation = args->op();
    op.m_message.Key = args->key();
    op.m_message.Value = args->value();
    op.m_message.ClientId = args->clientid();
    op.m_message.RequestId = args->requestid();

    int raftIndex = -1;
    int _ = -1;
    bool isLeader = false;

    m_raftNode->Start(op, &raftIndex, &_, &isLeader);       // put记录到raft的logs

    if(!isLeader)                                           // 当前Node不是leader，不能交互
    {
        DPrintf(
        "[func -KvServer::PutAppend -kvserver{%d}]From Client %s (Request %d) To Server %d, key %s, raftIndex %d , but "
        "not leader",
        m_me, &args->clientid(), args->requestid(), m_me, &op.m_message.Key, raftIndex);

        reply->set_err(ErrWrongLeader);
        return;
    }

    DPrintf(
      "[func -KvServer::PutAppend -kvserver{%d}]From Client %s (Request %d) To Server %d, key %s, raftIndex %d , is "
      "leader ",
      m_me, &args->clientid(), args->requestid(), m_me, &op.m_message.Key, raftIndex);


    // 拿出消息队列准备存储处理
    LockQueue<Op>* chForRaftIndex;
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if(waitApplyCh.find(raftIndex) == waitApplyCh.end())        // 每个raftIndex都有自己的消息队列，没有就创建
        {
            waitApplyCh.insert({raftIndex, new LockQueue<Op>});
        }

        chForRaftIndex = waitApplyCh[raftIndex];
    }

    Op raftCommitOp;

    if(!chForRaftIndex->TimeOutPop(CONSENSUS_TIMEOUT, &raftCommitOp))
    {
        DPrintf(
        "[func -KvServer::PutAppend -kvserver{%d}]TIMEOUT PUTAPPEND !!!! Server %d , get Command <-- Index:%d , "
        "ClientId %s, RequestId %s, Opreation %s Key :%s, Value :%s",
        m_me, m_me, raftIndex, &op.m_message.ClientId, op.m_message.RequestId,
        &op.m_message.Operation, &op.m_message.Key, &op.m_message.Value);

        if(IfRequestDuplicate(raftCommitOp.m_message.ClientId, raftCommitOp.m_message.RequestId))
        {
            reply->set_err(OK);     // 这里和get不同，如果超时但是重复（说明插入过这个key）为了幂等性，返回ok就行，不做插入操作
        }
        else
        {
            reply->set_err(ErrWrongLeader);     // 不是真正的错误Leader，
        }
    }
    else
    {
        DPrintf(
        "[func -KvServer::PutAppend -kvserver{%d}]WaitChanGetRaftApplyMessage<--Server %d , get Command <-- Index:%d , "
        "ClientId %s, RequestId %d, Opreation %s, Key :%s, Value :%s",
        m_me, m_me, raftIndex, &op.m_message.ClientId, op.m_message.RequestId,
        &op.m_message.Operation, &op.m_message.Key, &op.m_message.Value);

        if(raftCommitOp.m_message.ClientId == op.m_message.ClientId &&      // 在中间被覆盖了判断
            raftCommitOp.m_message.RequestId == op.m_message.RequestId)
        {
            reply->set_err(OK);
        }
        else
        {
            reply->set_err(ErrWrongLeader);
        }
    }

    {   // 临时的，用完删除
        std::lock_guard<std::mutex> lock(m_mutex);

        auto tmp = waitApplyCh[raftIndex];
        waitApplyCh.erase(raftIndex);
        delete tmp;
    }
}

/***********************************************************/
/**                      kvserver消息处理                   **/
/***********************************************************/

/// @brief 等待raft传来的消息，不是detech进程，是join，处理raft执行的操作到数据库
void KvServer::ReadRaftApplyCommandLoop()
{
    while(true)
    {
        auto message = applyChan->Pop();        // 内部加锁了，这个applychan是raft内部装载的未commit的log
        DPrintf(
            "---------------tmp-------------[func-KvServer::ReadRaftApplyCommandLoop()-kvserver{%d}] 收到了下raft的消息",
            m_me);

        if(message.CommandValid)
        {
            GetCommandFromRaft(message);
        }

        if(message.SnapshotVaild)
        {
            GetSnapShotFromRaft(message);
        }
    }
}


/// @brief  客户端与kvserver交互，交互之后存在kv数据库中。所以要拿出未提交的，就是数据库里的内容
/// @param message
void KvServer::GetCommandFromRaft(ApplyMsg message)
{
    Op op;
    op.parseFormString(message.Command);
    DPrintf(
      "[KvServer::GetCommandFromRaft-kvserver{%d}] , Got Command --> Index:{%d} , ClientId {%s}, RequestId {%d}, "
      "Opreation {%s}, Key :{%s}, Value :{%s}",
      m_me, message.CommandIndex, &op.m_message.ClientId, &op.m_message.RequestId,
        &op.m_message.Operation, &op.m_message.Key, &op.m_message.Value);

    if(message.CommandIndex <= m_lastSnapShotRaftLogIndex)          // 这个log已经被做成快照了
    {
        return;
    }

    if(!IfRequestDuplicate(op.m_message.ClientId, op.m_message.RequestId))  // 非重复
    {
        if(op.m_message.Operation == "Put")
        {
            ExecutePutOpOnKVDB(op);
        }

        if(op.m_message.Operation == "Append")
        {
            ExecuteAppendOpOnKVDB(op);
        }
    }

    if(m_maxRaftState != -1)    // 查询是不是超过最大的Logs了，超过了就做成快照
    {
        IfNeedToSendSnapShotCommand(message.CommandIndex, 9);
    }

    SendMessageToWaitChan(op, message.CommandIndex);
}

/// @brief 加入到waitApplyChan的等待通道，等待put或者get命令
/// @param op
/// @param raftIndex
/// @return 在waitApplyChan有没有找到这个通信的客户端id
bool KvServer::SendMessageToWaitChan(const Op &op, int raftIndex)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    DPrintf(
      "[RaftApplyMessageSendToWaitChan--> raftserver{%d}] , Send Command --> Index:{%d} , ClientId {%d}, RequestId "
      "{%d}, Opreation {%v}, Key :{%v}, Value :{%v}",
      m_me, raftIndex, &op.m_message.ClientId, op.m_message.RequestId, &op.m_message.Operation,
        &op.m_message.Key, &op.m_message.Value);

    if(waitApplyCh.find(raftIndex) == waitApplyCh.end())
        return false;

    waitApplyCh[raftIndex]->Push(op);
    DPrintf(
      "[RaftApplyMessageSendToWaitChan--> raftserver{%d}] , Send Command --> Index:{%d} , ClientId {%d}, RequestId "
      "{%d}, Opreation {%v}, Key :{%v}, Value :{%v}",
      m_me, raftIndex, &op.m_message.ClientId, op.m_message.RequestId, &op.m_message.Operation,
        &op.m_message.Key, &op.m_message.Value);
    return true;
}

/// @brief  如果超过最大的logs(state)大小，就做成快照
/// @param raftIndex    // raft当前commit的last index
/// @param proportion
void KvServer::IfNeedToSendSnapShotCommand(int raftIndex, int proportion)
{
    if(m_raftNode->GetRaftStateSize() > m_maxRaftState / 10.0)
    {
        auto snapshot = MakeSnapshot();         // 序列化跳表中数据
        m_raftNode->Snapshot(raftIndex, snapshot);  // 裁剪logs，已提交(commited)部分做成快照
    }
}

/// @brief
/// @return
std::string KvServer::MakeSnapshot()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string snapshotData = getSnapshotData();       // 在跳表中的dumpFile函数实现
    return snapshotData;
}

/// @brief
/// @param message
void KvServer::GetSnapShotFromRaft(ApplyMsg message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 里面一直返回true，不理解
    if(m_raftNode->CondInstallSnapshot(message.SnapshotTerm, message.SnapshotIndex, message.Snapshot))
    {
        ReadSnapshotToInstall(message.Snapshot);
        m_lastSnapShotRaftLogIndex = message.SnapshotIndex;
    }
}


//  raft会与persist层交互，kvserver层也会，因为kvserver层开始的时候需要恢复kvdb的状态
//  关于快照raft层与persist的交互：保存kvserver传来的snapshot；生成leaderInstallSnapshot RPC的时候也需要读取snapshot；
//  因此snapshot的具体格式是由kvserver层来定的，raft只负责传递这个东西
//  snapShot里面包含kvserver需要维护的persist_lastRequestId 以及kvDB真正保存的数据persist_kvdb
/// @brief kvserver获取快照进行加载到跳表
/// @param snapshot
void KvServer::ReadSnapshotToInstall(std::string snapshot)
{
    if(snapshot.empty())
    {
        return;
    }

    parseFromString(snapshot);
}