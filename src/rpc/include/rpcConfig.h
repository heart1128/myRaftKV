/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-04-30 17:41:36
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-04-30 18:03:51
 * @FilePath: /myRaftKv/src/rpc/include/mpRpcConfig.h
 * @Description: 
 */
#ifndef SRC_RPC_INCLUDE_RPCCONFIG_H
#define SRC_RPC_INCLUDE_RPCCONFIG_H

/*
    配置读取类
    1. 每个服务的节点信息，在config/node.config
*/

#include <string>
#include <unordered_map>

class MpRpcConfig{
public:
    // 解析加载配置文件
    void LoadConfigFile(const char* config_file);

    // 查询配置项
    std::string Load(const std::string& key);

private:
    // 存放加载的配置项
    std::unordered_map<std::string, std::string> m_configMap;
    // split去掉前后空格
    void Trim(std::string& src_str);
};

#endif // SRC_RPC_INCLUDE_MPRPCCONFIG_H