/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:43:36
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-01 20:16:18
 * @FilePath: /myRaftKv/src/common/util.cpp
 * @Description: 
 */
#include "util.h"

#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <functional>

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






void DPrintf(const char *format, ...)
{
    if(Debug)
    {
        // 获取当前时间
        time_t now = time(nullptr);
        tm* nowtm = localtime(&now);

        va_list args;
        va_start(args, format);
        std::printf("[%d-%d-%d-%d-%d-%d] ", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour,
                nowtm->tm_min, nowtm->tm_sec);
        std::vprintf(format, args);
        std::printf("\n");
        va_end(args);
    }
}