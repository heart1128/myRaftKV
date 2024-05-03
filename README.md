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
3. Clouser : service->CallMethod之后的调用，主要就是发送response

### 1.1.1 整体流程
客户端stub对象->CallMethod->序列化发送request->服务端启动->server反序列化request->调用重写的方法填充response->发送网络数据->客户端解析response->客户端stub对象取远程调用结果
1. 服务端：
    整个服务器使用muduo封装，只要实现rpc的逻辑业务jike 。
    - 服务注册：这里只是实现了在本地注册保存，用了一个map，使用的时候查询服务和方法
    - 连接请求：先拿到本机服务节点的ip:port，因为一个主机就是一个raft节点  
                通过muduo库进行TcpServer的创建，绑定连接回调和消息回调  
        - 连接回调：连接之前的回调函数，如果没有连接过就什么也不做等待连接，如果是旧连接还存在，就关闭（这样就是长连接，不用每个连接使用后立刻关闭）
        - 消息回调：在read到buffer之后的调用，主要就是做了对网络流数据proto反序列化，获取service调用CallMethod方法进行response填充。利用Clouser方法进行发送数据。  
2. 客户端:
    通过google::protobuf::RpcChannel重写CallMethod方法，等待stub实现调用。
    - 连接：connection直接连接服务端即可
    - 请求序列化：request方法使用proto序列化。设置上面三个参数。
    - 发送：通过send发送即可
    - response：发送之后等待服务端处理远程调用完毕，read读取反序列化到response，客户端就能通过stub取出远程调用的结果。


## 1.2、raft模块
### 1.2.1 超时选举过程
### 1.2.2 leader心跳、日志过程
