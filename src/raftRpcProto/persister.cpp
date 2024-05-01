/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-01 19:38:11
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-01 20:16:45
 * @FilePath: /myRaftKv/src/raftRpcProto/persister.cpp
 * @Description: 
 */
#include "persister.h"
#include "util.h"

/// @brief 初始化主要是初始化关闭文件状态和绑定文件到ostream流，之后可以追加读写
/// @param me 当前节点的编号，用作文件名区分
Persister::Persister(const int me)
        :m_raftStateFileName("raftStatePersist" + std::to_string(me) + ".txt"),
        m_snapshotFileName("snapShotFileName" + std::to_string(me) + ".txt"),
        m_raftStateSize(0)
{
    // 检查文件打开状态，清空文件
    bool fileOpenFlag = true;
    std::fstream file(m_raftStateFileName, std::ios::out | std::ios::trunc);
    if(file.is_open()) // 要关闭文件，之后绑定到ostream, 就是要确定文件存在，不存在就创建
    {
        file.close();
    }
    else
    {
        fileOpenFlag = false;
    }

    file = std::fstream(m_raftStateFileName, std::ios::out | std::ios::trunc);
    if(file.is_open()) // 要关闭文件，之后绑定到ostream
    {
        file.close();
    }
    else
    {
        fileOpenFlag = false;
    }

    if(!fileOpenFlag)
    {
        DPrintf("[func-Persister::Persister] file open error");
    }

    // 绑定ostream流
    m_raftStateOutStream.open(m_raftStateFileName);
    m_snapshotOutStream.open(m_snapshotFileName);
}

Persister::~Persister()
{
    // 关闭文件流
    if (m_raftStateOutStream.is_open()) {
        m_raftStateOutStream.close();
    }
    if (m_snapshotOutStream.is_open()) {
        m_snapshotOutStream.close();
    }
}

/// @brief 清空ostream，然后把传入的文件重新写入到流文件
/// @param raftState 
/// @param snapshot 
void Persister::Save(std::string raftState, std::string snapshot)
{
    // 必须上锁了，防止边写边持久化
    std::lock_guard<std::mutex> lock(m_mutex);
    // 先清空绑定的流，然后写入新的
    clearRaftStateAndSnapshot();

    m_raftStateOutStream << raftState;
    m_snapshotOutStream << snapshot;
}

/// @brief 单独保存raftState
/// @param data 
void Persister::SaveRaftState(const std::string &data)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    clearRaftState();
    m_raftStateOutStream << data;
    m_raftStateSize += data.size();
}

long long Persister::GetRaftStateSize()
{
    return m_raftStateSize;
}

std::string Persister::ReadSnapshot()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_snapshotOutStream.is_open())
    {
        m_snapshotOutStream.close();
    }
    
    // 延迟加载，等到文件读取完了，重新打开文件流
    DEFER {
        m_snapshotOutStream.open(m_snapshotFileName);
    };


    std::fstream ifs(m_snapshotFileName, std::ios_base::in);

    if(!ifs.good()){
        return "";
    }

    std::string snapshot;
    ifs >> snapshot;
    ifs.close();
    return snapshot;

    return std::string();
}

std::string Persister::ReadRaftState()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::fstream ifs(m_raftStateFileName, std::ios_base::in);
    if(!ifs.good()) // 打开检测，如果没有错放回true
    {
        return "";
    }

    std::string snapshot;
    ifs >> snapshot;
    ifs.close();
    return snapshot;
}

void Persister::clearRaftState()
{
    m_raftStateSize = 0;

    // 关闭
    if(m_raftStateOutStream.is_open())
    {
        m_raftStateOutStream.close();
    }

    // 关闭了要重新打开，一会要重新写入新的
    m_raftStateOutStream.open(m_raftStateFileName, std::ios::out | std::ios::trunc);
}

void Persister::clearSnapshot()
{
    // 关闭
    if(m_snapshotOutStream.is_open())
    {
        m_snapshotOutStream.close();
    }

    // 关闭了要重新打开，一会要重新写入新的
    m_snapshotOutStream.open(m_snapshotFileName, std::ios::out | std::ios::trunc);
}

void Persister::clearRaftStateAndSnapshot()
{
    clearRaftState();
    clearSnapshot();
}
