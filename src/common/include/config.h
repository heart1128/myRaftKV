/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 18:45:37
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-04 14:46:24
 * @FilePath: /myRaftKv/src/common/include/config.h
 * @Description: 
 */
#ifndef SRC_COMMON_INCLUDE_CONFIG_H
#define SRC_COMMON_INCLUDE_CONFIG_H
#include <string>

// 跳表相关
#define STORE_FILE "store/dumpFile"
static std::string delimiter = ":";

// 开启debug，有日志
const bool Debug = true;
const int debugMul = 1; // 时间单位：time.Millisecond，不同网络环境rpc速度不同，因此需要乘以一个系数
// 心跳发送时间
const int HeartBeatTimeout = 25 * debugMul;
// 超时随机生成时间，这里是设置最大最小
const int minRandomizedElectionTime = 300 * debugMul;  // ms
const int maxRandomizedElectionTime = 500 * debugMul;  // ms
// 发送数据给kvserver的间隔
const int ApplyInterval = 10 * debugMul;   



#endif