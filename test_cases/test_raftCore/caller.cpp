/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-10 10:40:20
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-12 16:46:55
 * @FilePath: /myRaftKv/test_cases/test_raftCore/caller.cpp
 * @Description: 
 */
#include <iostream>
#include "clerk.h"
#include "util.h"

int main()
{
    Clerk client;
    client.Init("node.conf");
    auto start = now();
    int count = 100;
    while(count--)
    {
        client.Put("x", std::to_string(count) + "测试");
        std::string get1 = client.Get("x");
        std::printf("get return :{%s}\r\n", get1.c_str());
    }
    return 0;
}