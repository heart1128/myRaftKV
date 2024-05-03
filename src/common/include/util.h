/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:43:47
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-02 21:32:41
 * @FilePath: /myRaftKv/src/common/include/util.h
 * @Description: 
 */

#ifndef SRC_COMMON_INCLUDE_UTIL_H
#define SRC_COMMON_INCLUDE_UTIL_H

#include "config.h"
#include "kvServer.pb.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <random>

// 当前时间
std::chrono::_V2::system_clock::time_point now()
{
    return std::chrono::high_resolution_clock::now();
}
// 日志保存
void DPrintf(const char* format, ...);
void myAssert(bool condition, std::string message);

/***********************************************************/
/**                     接收无限个参数输出 可变参数模板  **/
/***********************************************************/
template<typename... Args>
std::string format(const char* fomat_str, Args... args)
{
    std::stringstream ss;
    // ((ss << args), 0) 写入参数到string流中，0中间是,号，确保整个表达式结果是0
    // ((ss << args), 0)... 利用数组初始化的std::initializer会不断展开初始化，直到所有的args...都写入流中
    int _[] = {((ss << args), 0)...};
    (void)_; // _[]声明未使用，所以这里做一个无效空操作让编译器不警告
    return ss.str();
}

/***********************************************************/
/**                     获取随机事件，用来注册follower的超时时间   **/
/***********************************************************/

std::chrono::microseconds getRandomizeElectionTimeout()
{
    // linux下使用
   std::random_device rd;
   std::mt19937 rng(rd());
   // 用正态分布生成
   std::uniform_int_distribution<int> dist(minRandomizedElectionTime, maxRandomizedElectionTime);
   return std::chrono::milliseconds(dist(rng));
}

/***********************************************************/
/**                     kvserver和raft交互类            **/
/***********************************************************/
struct Message{
    std::string Operation; // 三个操作 Get Put Append
    std::string Key;
    std::string Value;
    std::string ClientId; // 客户端ID
    int RequestId; // 请求的request序列号（为了保证线性一致性）防止request的顺序出错
};
/// @brief kv发送给raft的command，经过protobuf序列化的
class Op{
public:
    // TODO
    // 用了protobuf的序列化，要定义文件了
    /// @brief 序列化到string，发送的
    std::string asString() {
        std::string serialize;
        
        m_KvMessage.set_operation(m_message.Operation.c_str());
        m_KvMessage.set_key(m_message.Key.c_str());
        m_KvMessage.set_value(m_message.Value.c_str());
        m_KvMessage.set_clientid(m_message.ClientId.c_str());
        m_KvMessage.set_requestid(m_message.RequestId);

        if(!m_KvMessage.SerializeToString(&serialize))
        {
            std::cout << "kvServer data serialize to string error!" << std::endl;
            return "";
        }
        return serialize;
    }
    /// @brief  反序列化到message
    /// @param message 
    /// @return 
    bool parseFormString(std::string message)
    {
        if(!m_KvMessage.ParseFromString(message))
        {
            std::cout << "kvServer data parse to string error!" << std::endl;
            return false;
        }

        m_message.Operation = m_KvMessage.operation();
        m_message.Key = m_KvMessage.key();
        m_message.Value = m_KvMessage.value();
        m_message.ClientId = m_KvMessage.clientid();
        m_message.RequestId = m_KvMessage.requestid();
        return true;
    }

private:
    Message m_message;
    KVServer::KVMessage m_KvMessage;
};

/***********************************************************/
/**                     延迟类函数                        **/
/***********************************************************/
// 延迟类，在函数的return之后执行
/// @brief 原理就是保存函数，宏定义声明对象，等待函数退出解析函数执行
/// @tparam F 
template <class F>
class DeferClass{
public:
    // 完美转发，支持所有类型
    // 注意，这里没有用explic ，因为一会要用 = 进行隐式转换
    DeferClass(F&& f):m_func(std::forward<F>(f)){}
    DeferClass(const F& f) : m_func(f){}
    ~DeferClass(){ m_func();}

    // 这里需要禁止拷贝构造和重载= ，因为上面需要=调用构造隐式转换
    DeferClass(const DeferClass& e) = delete;
    DeferClass& operator=(const DeferClass& e) = delete;

private:
    F m_func;
};

// 扩展就是DeferClass defer_placeholderline = [&]()
#define _CONCAT(a, b) a##b
#define _MAKE_DEFER_(line) DeferClass _CONCAT(defer_placeholder, line) = [&]()
#undef DEFER
#define DEFER _MAKE_DEFER_(__LINE__)


/***********************************************************/
/**                     有锁队列                       **/
/***********************************************************/
/// @brief 异步写日志的队列，有锁的，用queue模拟的
/// @tparam T 
template<typename T>
class LockQueue{
public:
    /// @brief 多个worker线程会同时写，要加锁。
    /// @param data 
    void Push(const T& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condition_variable.notify_one();// 有任务进入，可以消费了
    }

    /// @brief 拿出队列一个数据，如果队列为空就要用条件变量睡眠
    /// @return T类型的data
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        while(m_queue.empty())
        {
            m_condition_variable.wait(lock);
        }

        // 不为空拿数据
        T data = m_queue.front();
        m_queue.pop();
        lock.unlock();
        return data;
    }

    /// @brief 定时取数据，设置超时时间
    /// @param timeout 
    /// @param ResData 
    /// @return 是否拿到了data
    bool TimeOutPop(int timeout, T* ResData)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // 1. 计算超时时间
        auto now = std::chrono::system_clock::now();
        auto timeout_time = now + std::chrono::milliseconds(timeout);

        // 2. 在超时之前不断循环，直到为空
        while(m_queue.empty())
        {
            // 设置超时判断
            if(m_condition_variable.wait_until(lock, timeout_time) == std::cv_status::timeout)
                return false;
            else
                continue;
        }

        T data = m_queue.front();
        m_queue.pop();
        *ResData = data;
        return true;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition_variable; // 有任务就唤醒的条件变量
};

#endif
