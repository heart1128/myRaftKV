#include "include/rpcConfig.h"

#include <iostream>
#include <string>



void MpRpcConfig::LoadConfigFile(const char* config_file)
{
    FILE *pf = fopen(config_file, "r");

    if(nullptr == pf)
    {
        // 先用cout 再看怎么升级,kafka也行
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

// 1.注释   2.正确的配置项 =    3.去掉开头的多余的空格
  while (!feof(pf)) {
    char buf[512] = {0};
    fgets(buf, 512, pf);

    // 去掉字符串前面多余的空格
    std::string read_buf(buf);
    Trim(read_buf);

    // 判断#的注释
    if (read_buf[0] == '#' || read_buf.empty()) {
      continue;
    }

    // 解析配置项
    int idx = read_buf.find('=');
    if (idx == -1) {
      // 配置项不合法
      continue;
    }

    std::string key;
    std::string value;
    key = read_buf.substr(0, idx);
    Trim(key);
    // rpcserverip=127.0.0.1\n
    int endidx = read_buf.find('\n', idx);
    value = read_buf.substr(idx + 1, endidx - idx - 1);
    Trim(value);
    m_configMap.insert({key, value});
  }

  fclose(pf);
}

std::string MpRpcConfig::Load(const std::string& key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) {
        return "";
    }
    return it->second;
}

void MpRpcConfig::Trim(std::string& src_str)
{
     int idx = src_str.find_first_not_of(' ');
        if (idx != -1) {
            // 说明字符串前面有空格
            src_str = src_str.substr(idx, src_str.size() - idx);
        }
        // 去掉字符串后面多余的空格
        idx = src_str.find_last_not_of(' ');
        if (idx != -1) {
            // 说明字符串后面有空格
            src_str = src_str.substr(0, idx + 1);
        }
}