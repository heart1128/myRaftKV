/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:43:36
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-04-30 18:47:46
 * @FilePath: /myRaftKv/src/common/util.cpp
 * @Description: 
 */
#include "util.h"

#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstdarg>

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