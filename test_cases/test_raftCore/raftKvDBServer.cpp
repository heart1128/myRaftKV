/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-10 10:40:31
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-11 21:01:27
 * @FilePath: /myRaftKv/test_cases/test_raftCore/raftKvDBServer.cpp
 * @Description: 
 */
#include <iostream>
#include "raft.h"
#include "kvServer.h"
#include <unistd.h>
#include <random>
#include <memory>

void showArgsHelp()
{
    std::cout << "format: command -n <nodeNum> -f <configFileName>" << std::endl;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    int nodeNum = 0;

    std::string configFileName;

    std::random_device rd;
    std::mt19937 gen(rd()); //   随机生成器
    std::uniform_int_distribution<> dis(10000, 29999);  // 分布生成端口10000 - 29999的

    unsigned short startPort = dis(gen);     

    // 1. 拿到命令行参数，节点数量和节点文件
    while((c = getopt(argc, argv, "n:f:")) != -1)       // 解析参数，-n和-f之后的n:表示n后面还要接参数
    {
        switch (c)
        {
        case 'n':   // 处理-n
            nodeNum = atoi(optarg); // 外部变量，接收每次解析的opt
            break;
        case 'f':
            configFileName = optarg;
            break;
        default:
            showArgsHelp();
            exit(EXIT_FAILURE);
        }
    }  

    // 2. 重定向节点文件到文件流

    std::ofstream file(configFileName, std::ios::out | std::ios::app);
    file.close();   // 如果不存在就先创建空的，如果存在什么也不做，只是打开了下

    file = std::ofstream(configFileName, std::ios::out | std::ios::trunc); // 清空文件     
    if(file.is_open())
    {
        file.close();
        std::cout << configFileName << "已清空" << std::endl;
    }
    else
    {
        std::cout << "无法打开 " << configFileName << std::endl;
        exit(EXIT_FAILURE);
    }

    // 每个节点开启kvServer
    for(int i = 0; i < nodeNum; ++i)
    {
        short port = startPort + static_cast<short>(i); // 用的随机port

        pid_t pid = fork();
        if(pid == 0) // 子进程
        {
            std::cout << "start to create raftkv node:" << i << "    port:" << port << " pid:" << getpid() << std::endl;
            std::shared_ptr<KvServer> kvServer =
                        std::make_shared<KvServer>(i, 500, configFileName, port);
            pause();    // 子进程等待
        }
        else if(pid > 0)    // 父进程
        {
            sleep(1);
        }
        else // 进程创建失败
        {
            std::cerr << "Failed to create child process." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    pause();
    return 0;
}