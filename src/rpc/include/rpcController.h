/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:07:33
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-04-30 18:08:03
 * @FilePath: /myRaftKv/src/rpc/include/mpRpcController.h
 * @Description: 
 */
#ifndef SRC_RPC_INCLUDE_RPCCONTRILLER_H
#define SRC_RPC_INCLUDE_RPCCONTRILLER_H

#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController {
 public:
  MprpcController();
  void Reset();
  bool Failed() const;
  std::string ErrorText() const;
  void SetFailed(const std::string& reason);

  // 目前未实现具体的功能
  void StartCancel();
  bool IsCanceled() const;
  void NotifyOnCancel(google::protobuf::Closure* callback);

 private:
  bool m_failed;          // RPC方法执行过程中的状态
  std::string m_errText;  // RPC方法执行过程中的错误信息
};


#endif