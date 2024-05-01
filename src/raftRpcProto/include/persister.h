/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-01 19:37:40
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-01 20:00:43
 * @FilePath: /myRaftKv/src/raftRpcProto/include/persister.h
 * @Description: 
 */
#ifndef SRC_RAFTRPCPROTO_INCLUDE_PERSISTER_H
#define SRC_RAFTRPCPROTO_INCLUDE_PERSISTER_H

// 分为state持久化和snapshot持久化
// 主要就是使用ostream流追加
// 记得上锁

#include <fstream>
#include <mutex>
#include <string>

class Persister{
public:
    explicit Persister(const int me);
    ~Persister();

public:
    // 两种持久化一起写入
    void Save(std::string raftState, std::string snapshot);
    // 单独保存state
    void SaveRaftState(const std::string& data);

    long long GetRaftStateSize();

    std::string ReadSnapshot();
    std::string ReadRaftState();

private:
    void clearRaftState();
    void clearSnapshot();
    void clearRaftStateAndSnapshot();

private:
    std::mutex m_mutex;
    std::string m_raftState;
    std::string m_snapshot;

    std::string m_raftStateFileName; // state保存的文件名
    std::string m_snapshotFileName; // 快照保存的文件名
    
    std::ofstream m_raftStateOutStream; // state的输出文件流
    std::ofstream m_snapshotOutStream; // snapshot的输出文件流

    long long m_raftStateSize; // state的大小
};



#endif // SRC_RAFTRPCPROTO_INCLUDE_PERSISTER_H