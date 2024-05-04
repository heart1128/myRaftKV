/*
 * @Author: heart1128 1020273485@qq.com
 * @Date: 2024-05-04 14:35:11
 * @LastEditors: heart1128 1020273485@qq.com
 * @LastEditTime: 2024-05-04 20:33:05
 * @FilePath: /myRaftKv/src/skipList/include/skipList.h
 * @Description: 
 */
#ifndef SRC_SKIPLIST_INCLUDE_SKIPLIST_H
#define SRC_SKIPLIST_INCLUDE_SKIPLIST_H

#include <memory>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <mutex>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "config.h"
#include "skipListDump.pb.h"

/***********************************************************/
/**                      跳表Node                         **/
/***********************************************************/

template<typename K, typename V>
class SkipListNode{
public:
    typedef std::shared_ptr<SkipListNode<K, V>> ptr;    
public:
    SkipListNode(){}
    SkipListNode(K k, V v, int level);
    ~SkipListNode();

public:
    K getKey() const;
    V getValue() const;

    void setValue(V value);

public:
    // struct skipListLevel
    // {
    //     SkipListNode<K, V> *forward;    // 指向不同层级下一个Node的指针
    //     unsigned log nodeLevel;         // 当前node的层级
    // }level[];
    
    SkipListNode<K, V> **forward;          // 每个节点都会放在每一层，相当于*forward[level]指针数组，每一层存放一个指向下一个的指针
    int nodeLevel;

private:
    K m_key;
    V m_value;
};


template <typename K, typename V>
inline SkipListNode<K, V>::SkipListNode(K k, V v, int level)
{
    m_key = k;
    m_value = v;
    nodeLevel = level;

    forward = new SkipListNode<K, V>* [level + 1];      // 一共level+1层， 0 - level

    memset(forward, 0, sizeof(SkipListNode<K, V>*) * (level + 1));
}

template <typename K, typename V>
inline SkipListNode<K, V>::~SkipListNode()
{
    delete[] forward;
}

template <typename K, typename V>
inline K SkipListNode<K, V>::getKey() const
{
    return m_key;
}

template <typename K, typename V>
inline V SkipListNode<K, V>::getValue() const
{
    return m_value;
}

template <typename K, typename V>
inline void SkipListNode<K, V>::setValue(V value)
{
    m_value = value;
}

/***********************************************************/
/**                      kv持久化                          **/
/***********************************************************/
template<typename K, typename V>
class SkipListDump{
public:
    void insert(const SkipListNode<K, V>& node);
    void serializeToString(std::string& data);
    void loadFromSerializedString(const std::string& data);
public:
    std::vector<K> m_keyDump;
    std::vector<V> m_valueDump;
};

template <typename K, typename V>
inline void SkipListDump<K, V>::insert(const SkipListNode<K, V> &node)
{
    m_keyDump.emplace_back(node->getKey());
    m_valueDump.emplace_back(node->getValue());
}

template <typename K, typename V>
inline void SkipListDump<K, V>::serializeToString(std::string &data)
{
    std::shared_ptr<SkipListDumpSerialization::KVDump> kvDump = 
                        std::make_shared<SkipListDumpSerialization::KVDump>();
    for(int i = 0; i < m_keyDump.size(); ++i)
    {
        kvDump->mutable_key()->Add(m_keyDump[i]);
        kvDump->mutable_value()->Add(m_valueDump[i]);
    }

    if(!kvDump->SerializeToString(&data))
    {
        std::cout << "序列化存储失败！" << std::endl;
        data = "";
    }
}

template <typename K, typename V>
inline void SkipListDump<K, V>::loadFromSerializedString(const std::string &data)
{
    std::shared_ptr<SkipListDumpSerialization::KVDump> kvDump = 
                        std::make_shared<SkipListDumpSerialization::KVDump>();
    
    if(!kvDump->ParseFromString(data))
    {
        std::cout << "序列化解析失败！" << std::endl;
        return;
    }

    for(int i = 0; i < kvDump->key_size(); ++i)
    {
        m_keyDump.emplace_back(kvDump->key(i));
        m_valueDump.emplace_back(kvDump->value(i));
    }
}

/***********************************************************/
/**                      跳表                             **/
/***********************************************************/
template<typename K, typename V>
class SkipList{
public:
    SkipList(int maxLevel);
    ~SkipList();

public:
    int getRandomLevel();
    SkipListNode<K, V>* createNode(K key, V value, int level);
    int insertElement(K key, V value);
    void displayList();
    bool searchElement(K key, V& value);
    void deleteElement(K key);
    void insertSetElement(K& key, V& value);
    std::string dumpFile();
    void loadFile(const std::string& dumStr);
    void clear(SkipListNode<K, V>*);            // 递归删除节点，删除跳表
    int size();

private:
    void getKeyAndValueFromString(const std::string& str, std::string& key, std::string& value);
    bool isValidString(const std::string& str);

private:
    int m_maxLevel;
    int m_skipListLevel;

    SkipListNode<K, V>* m_header;               
    // std::shared_ptr<SkipListNode<K, V>> m_header;               // 管理整个跳表 层数数组

    std::ofstream m_fileWriter;
    std::ifstream m_fileReader;

    int m_elementCount;
    std::mutex m_mutex;
};

template <typename K, typename V>
inline SkipList<K, V>::SkipList(int maxLevel)
{
    m_maxLevel = maxLevel;
    m_skipListLevel = 0;
    m_elementCount = 0;

    K key;
    V value;
    // header管理整个跳表
    m_header = new SkipListNode<K, V>(key, value, maxLevel);
    // m_header = std::make_shared<SkipListNode<K, V>>(key, value, maxLevel);
}

template <typename K, typename V>
inline SkipList<K, V>::~SkipList()
{
    if(m_fileReader.is_open())
    {
        m_fileReader.close();
    }
    if(m_fileWriter.is_open())
    {
        m_fileWriter.close();
    }

    if(m_header->forward[0] != nullptr)
    {
        clear(m_header->forward[0]);
    }
    // delete m_header;         
    // m_header = nullptr;
}

template <typename K, typename V>
inline int SkipList<K, V>::getRandomLevel()
{
    int k = 1;
    while(rand() % 2)
    {
        ++k;
    }
    k = std::min(m_maxLevel, k);
    return k;
}

/// @brief
/// @tparam K
/// @tparam V
/// @param key
/// @param value
/// @param level
/// @return
template <typename K, typename V>
inline SkipListNode<K, V>* SkipList<K, V>::createNode(K key, V value, int level)
{
   SkipListNode<K, V>* skipListNode = new SkipListNode<K, V>(key, value, level);
   return skipListNode;
}

template<typename K, typename V>
inline void SkipList<K, V>::clear(SkipListNode<K, V>* node)
{
    if(node->forward[0] != nullptr)
    {
        clear(node->forward[0]);
    }
    delete node;
}


/*
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

/// @brief 寻找每一层应该插入的位置，每一层都要插入这个新节点，如果是已经存在的key就不插入
/// @tparam K 
/// @tparam V 
/// @param key 
/// @param value 
/// @return 返回1表示key已存在，返回0表示插入成功
template <typename K, typename V>
inline int SkipList<K, V>::insertElement(K key, V value)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    SkipListNode<K, V>* current = m_header;             // 从header开始查找

    SkipListNode<K, V>* update[m_maxLevel + 1];         // 记录每一层应该插入的节点（prev节点）    
    memset(update, 0, sizeof(SkipListNode<K, V>*) * (m_maxLevel + 1));

    /// 1. 寻找每层要插入的位置
    for(int i = m_skipListLevel; i >= 0; --i)           // 从当前最高level开始找，比较大小，当前节点小就往前走，如果前面没有节点了，就往下level走
    {
        // 找当前层 1. 后面一个节点不为空 2. 后面的节点Key < 指定key一直往下走。跳出就是换层
        while(current->forward[i] != nullptr && current->forward[i]->getKey() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;                            // 记录层换层的最后一个节点，位置就是这些节点后面(每层都要插入新的节点)
    }

    current = current->forward[0];                      // 当前已经是最下面一层了，当节点的下一个就是新key的插入点
    /// 2. 存在的key不插入
    if(current != nullptr && current->getKey() == key)  // 下一个要插入点的key和新key相同，已存在就不插入
    {
        std::cout << "key:" << key << ", exists" << std::endl;
        lock.unlock();
        return 1;
    }

    /// 3. 生成随机层插入（插入随机层，不是每层）
    if(current == nullptr || current->getKey() != key)
    {
        int random_level = getRandomLevel();            // 跳表插入不是固定层插入的，是随机化选择层插入，防止插入堆积在一层 https://blog.csdn.net/qq_56870066/article/details/127463972
        if(random_level > m_skipListLevel)              // 随机插入的层数目前没有创建（随机层是在0-max_level之间），所以创建空层，只要一个header
        {
            for(int i = m_skipListLevel + 1; i <= random_level; ++i)
            {
                update[i] = m_header;
            }

            m_skipListLevel = random_level;
        }

        SkipListNode<K, V>* insertedNode = createNode(key, value, random_level);
        for(int i = 0; i <= random_level; ++i)
        {
            insertedNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = insertedNode;       // 随机数的每层都插入新节点

            /*  上面的操作，每一层的
            node->next = head->next;
            head->next = ndoe;
            */
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        ++m_elementCount;
    }
    return 0;
}

template <typename K, typename V>
inline void SkipList<K, V>::displayList()
{
    std::cout << "\n*****Skip List*****" << "\n";
    for(int i = 0; i <= m_skipListLevel; ++i)
    {
        SkipListNode<K, V>* node = m_header->forward[i];    // 取出第i层

        std::cout << "Level" << i << ":";
        while(node != nullptr)
        {
            std::cout << node->getKey() << ":" << node->getValue() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

/// @brief 根据key和value每层查找元素
/// @tparam K 
/// @tparam V 
/// @param key 
/// @param value 
/// @return 是否找到
template <typename K, typename V>
inline bool SkipList<K, V>::searchElement(K key, V &value)
{
    SkipListNode<K, V>* current = m_header;         // header就是最上面一层

    for(int i = m_skipListLevel; i >=0 ; --i)
    {
        while(current->forward[i] && current->forward[i]->getKey() < key)
        {
            current = current->forward[i];
        }
    }

    current = current->forward[0];                  // 第0层

    if(current and current->getKey() == key)
    {
        value = current->getValue();
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

/// @brief 记录每层找到的key的前一个节点，找到了保存，然后删除每层的单链表节点即可
/// @tparam K
/// @tparam V
/// @param key
template <typename K, typename V>
inline void SkipList<K, V>::deleteElement(K key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    SkipListNode<K, V>* current = m_header;
    SkipListNode<K, V>* update[m_skipListLevel + 1];        // 删除不添加层，用当前层数就行
    memset(update, 0, sizeof(SkipListNode<K, V> *) * (m_skipListLevel + 1));

    // 1. 查找每层该节点所在的前一个节点
    for(int i = m_skipListLevel; i >= 0; --i)
    {
        while(current->forward[i] != nullptr && current->forward[i]->getKey() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }


    // 2. 循环删除存在层的key
    current = current->forward[0];      // 当前肯定是0层，current此时就是要删除的节点

    if(current != nullptr && current->getKey() == key)
    {
        for(int i = 0; i <= m_skipListLevel; ++i)
        {
            if(update[i]->forward[i] != current)            // 这种情况就是随机插入的时候到这层停止了，（插入的时候一定会从0层开始到randomLevel）
                break;                                      // 所以这个层就是randmoLevel层，再上面的层都是没有插入的层
            
            update[i]->forward[i] = current->forward[i];    // 等同于链表节点删除 prev->next = cur->next
        }

        while(m_skipListLevel > 0 && m_header->forward[m_skipListLevel] == 0)   // 最上面一层如果删除之后节点为0，就要删除一层
        {
            --m_skipListLevel;
        }

        std::cout << "Successfully deleted key " << key << std::endl;
        delete current;
        current = nullptr;
        --m_elementCount;
    }
}

/// @brief 和insert的区别就是如果已经存在key，就变更值不是不插入(这里的操作是先找，找到删除重新插入)
/// @tparam K 
/// @tparam V 
/// @param key 
/// @param value 
template <typename K, typename V>
inline void SkipList<K, V>::insertSetElement(K &key, V &value)
{
    V oldValue;
    if(searchElement(key, oldValue))
    {
        deleteElement(key);
    }
    insertElement(key, value);
}

/// @brief 返回跳表中元素的个数
/// @tparam K 
/// @tparam V 
/// @return 返回跳表中元素的个数
template <typename K, typename V>
inline int SkipList<K, V>::size()
{
    return m_elementCount;
}

/// @brief 输入 key:value的字符串解析进行查找
/// @tparam K 
/// @tparam V 
/// @param str 
/// @param key 
/// @param value 
template <typename K, typename V>
inline void SkipList<K, V>::getKeyAndValueFromString(const std::string &str, std::string &key, std::string &value)
{
    if(!isValidString(str))
        return;
    
    int delimiterIndex = str.find(delimiter);
    key = str.substr(0, delimiterIndex);
    value = str.substr(delimiterIndex, str.size());
}

/// @brief 单纯的检查字符串是不是空，有没有":"分隔符
/// @tparam K 
/// @tparam V 
/// @param str 
/// @return 字符串解析是否正确
template <typename K, typename V>
inline bool SkipList<K, V>::isValidString(const std::string &str)
{
    if(str.empty())
        return false;
    
    if(str.find(delimiter) == str.npos)
        return false;
    
    return true;
}

// 持久化
template <typename K, typename V>
inline std::string SkipList<K, V>::dumpFile()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    SkipListNode<K, V>* node = m_header->forward[0];
    SkipListDump<K, V> dumper;

    while(node != nullptr)
    {
        dumper.insert(*node);
        node = node->forward[0];
    }

    std::string data;
    dumper.serializeToString(data);
    return data;
}


template <typename K, typename V>
inline void SkipList<K, V>::loadFile(const std::string &dumpStr)
{
    if(dumpStr.empty())
    {
        std::cout << "加载的数据文件为空" << std::endl;
        return;
    }

    SkipListDump<K, V> dumper;

    for(int i = 0; i < dumper.m_keyDump.size(); ++i)
    {
        insertElement(dumper.m_keyDump[i], dumper.m_valueDump[i]);
    }
}

#endif

