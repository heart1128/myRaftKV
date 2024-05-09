#ifndef SRC_RAFTCLERK_INCLUDE_RAFTSERVERRPCUTIL_H
#define SRC_RAFTCLERK_INCLUDE_RAFTSERVERRPCUTIL_H

#include <iostream>

#include "kvServerRpc.pb.h"
#include "rpcChannel.h"
#include "rpcController.h"

class raftServerRpcUtil
{
public:
    raftServerRpcUtil(std::string ip, short port);

public:

    bool PutAppend(const ::raftKVRpcProctoc::PutAppendArgs *request, raftKVRpcProctoc::PutAppendReply *response);

    bool Get(const ::raftKVRpcProctoc::GetArgs *request, raftKVRpcProctoc::GetReply *response);

private:
    std::shared_ptr<raftKVRpcProctoc::KvServerRpc_Stub> m_stub;
};





#endif // SRC_RAFTCLERK_INCLUDE_RAFTSERVERRPCUTIL_H