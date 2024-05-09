#include "raftServerRpcUtil.h"


raftServerRpcUtil::raftServerRpcUtil(std::string ip, short port)
{
    m_stub = std::make_shared<raftKVRpcProctoc::KvServerRpc_Stub>(new RpcChannel(ip, port, false));
}

bool raftServerRpcUtil::PutAppend(const ::raftKVRpcProctoc::PutAppendArgs *request, raftKVRpcProctoc::PutAppendReply *response)
{
    MprpcController controller;
    m_stub->PutAppend(&controller, request, response, nullptr);
    return !controller.Failed();
}

bool raftServerRpcUtil::Get(const ::raftKVRpcProctoc::GetArgs *request, raftKVRpcProctoc::GetReply *response)
{
    MprpcController controller;
    m_stub->Get(&controller, request, response, nullptr);
    return !controller.Failed();
}
