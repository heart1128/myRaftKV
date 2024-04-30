# 说明
## 本仓库用于学习raft分布式共识算法


# 1、模块说明
## 1.1、RPC模块
rpc的头部部分使用proto编写，三个参数
``` shell
message RpcHeader
{
    bytes service_name = 1;  // 服务名
    bytes method_name = 2;   // 服务方法
    uint32 args_size = 3;    
}
```
protobuf要实现的部分是:
1. channel：客户端调用，传入参数
2. controller : 传输控制参数