#include "memtable.h"

/**
 * @param TIMESTAMP 初始的时间戳
*/

MemTable::MemTable(int TIMESTAMP)
{
    time_stamp = TIMESTAMP;

    //构造bloom过滤器
    bloom_filter = new BloomFilter(BLOOM_FILTER_BYTE);
    
    //构造跳表
    skip_list = new skiplist::SkipList();
}

MemTable::~MemTable()
{
    delete bloom_filter;
    delete skip_list;
}

void MemTable::generate_header(char *start, uint64_t time_stamp, uint64_t key_val_count, key_t min, key_t max)
{
    *((uint64_t *)start) = time_stamp;
    start += 8;//time_stamp -> 64 bit

    *((uint64_t *)start) = key_val_count;
    start += 8; // key_val_count -> 64 bit

    *((uint64_t *)start) = min;
    start += 8; // min_key -> 64 bit

    *((uint64_t *)start) = max;
}

void MemTable::generate_bloomfilter(char *start)
{
    std::memcpy(start, bloom_filter->getFilterString(), BLOOM_FILTER_BYTE);
}

void MemTable::generate_triple(char *start, key_t key, offset_t offset, vlen_t vlen) 
{
    *((uint64_t *)start) = key;
    start += 8; // key -> 64 bit

    *((uint64_t *)start) = offset;
    start += 8; // offset -> 64 bit

    *((uint32_t *)start) = vlen;//vlen -> 32 bit
}

void MemTable::generate_vlog_entry(std::string &vlog_entry_str,key_t key,vlen_t vlen,const val_t& val)
{
    //写入vlog条目中的 magic check key vlen 部分
    char buffer[VLOG_MAGIC_BYTE + VLOG_CHECK_BYTE + VLOG_KEY_BYTE + VLOG_VLEN_BYTE];
    char *p = buffer;
    
    //写入magic部分
    *((magic_t *)p) = MAGIC_SIGN;

    //check部分等待之后生成
    //check_ptr记录下要写check的位置
    p += VLOG_MAGIC_BYTE;
    char *check_ptr = p;

    //写入key
    p += VLOG_CHECK_BYTE;
    *((key_t *)p) = key;

    //写入vlen
    p += VLOG_KEY_BYTE;
    *((vlen_t *)p) = vlen;

    //check_str用来给crc16生成校验
    std::string check_str;
    
    check_str.append(buffer + VLOG_MAGIC_BYTE + VLOG_CHECK_BYTE, VLOG_KEY_BYTE + VLOG_VLEN_BYTE);
    check_str.append(val);

    //转换为vector<unsigned char>
    std::vector<unsigned char> v(check_str.begin(), check_str.end());

    v.push_back('\0');
    check_t check = utils::crc16(v);
    
    *((check_t *)check_ptr) = check;

    //将vlog的条目真正写入传进来的vlog_entry_str中
    vlog_entry_str.append(buffer, VLOG_MAGIC_BYTE + VLOG_CHECK_BYTE + VLOG_KEY_BYTE + VLOG_VLEN_BYTE);
    vlog_entry_str.append(val);

    return;
}

/**
 * @param path 要创建新的SSTable的位置
 * @param offset_start 在vlog中这块数据的起始偏移位置
*/
void MemTable::createSSTable(const std::string &path, offset_t offset_start, std::string &vlog_entry)
{
    // 优化?
    //清空用来写sstable中内容的字符串缓冲区
    memset(sstable_buffer, 0x0, 20000);
    
    //获取写sstable中head bf triple三元组的起始位置
    char *head_start = sstable_buffer;
    char *bloom_filter_start = head_start + HEADER_BYTE;
    char *triple_start = bloom_filter_start + BLOOM_FILTER_BYTE;

    //获取跳表中的节点
    skiplist::skipNode *skip_node_ptr = this->skip_list->getHeader()->forward[1];
    skiplist::skipNode *tail = this->skip_list->getTail();

    // 记录最大和最小的键
    key_t max_key;
    key_t min_key = skip_node_ptr->key;
    
    //循环中的键、偏移量、长度
    key_t key;
    offset_t offset = offset_start;
    vlen_t vlen;

    //记录实际中有多少个元素
    int count = 0;

    // 写三元组部分
    while (skip_node_ptr != tail){//直到尾节点停止
        
        count ++;//数据量++

        key = skip_node_ptr->key;          // 获取键值
        const val_t &val = skip_node_ptr->val;//获取值
        vlen = val.length();   // 获取字符串长度

        generate_triple(triple_start, key, offset, vlen);//写入三元组

        generate_vlog_entry(vlog_entry, key, vlen, val);//将生成的vlog中数据条目写入vlog_entry末尾

        skip_node_ptr = skip_node_ptr->forward[1];

        offset += 1 + 2 + 8 + 4 + vlen;//计算vlog中下一条entry的起始偏移量
        triple_start += TRIPLE_BYTE;//缓冲区中要写的下一条三元组的起始位置
    }

    max_key = key;

    if (count == 0){
        //corner case
    }

    generate_header(head_start, ++time_stamp, count, min_key, max_key);

    generate_bloomfilter(bloom_filter_start);

    std::ofstream file(path);
    if (file.is_open()){
        //file << this->sstable_buffer;
        file.write(this->sstable_buffer, HEADER_BYTE + BLOOM_FILTER_BYTE + TRIPLE_BYTE * count);
        file.close();
    }
}


std::string MemTable::naive_createSSTable()
{
}

/**
 * @brief 
 * 在内存中的memtable跳表中插入键值对
 * 
 * @param key 要插入的键
 * @param val 要插入的值
 * @return true 插入成功
 * @return false 插入失败，表示已经超额，需要生成sstable
 */
bool MemTable::put(key_t key,const val_t &val)
{
    //如果再插入一个就超额了 就需要创建sstable并写入vlog中
    if (this->skip_list->isFull())
        return 0;

    this->skip_list->put(key, val);
    this->bloom_filter->insert(key);

    return 1;
}

/**
 * @brief 
 * 在内存中的memtable跳表中删除键值对
 * 删除不会讲节点从跳表中删除
 * 而是对已经存在的键值对将值覆写成~DELETE~删除标记
 * 记录的节点数也不会减少
 * 
 * 如果不存在这个键，就会返回false
 * 告诉外部memtable中找不到要删除的键，得去磁盘中寻找
 * @param key 要删除的键
 * @return true 删除成功，在跳表中添加键的~DELETE~
 * @return false 删除失败，因为没找到键
 */
bool MemTable::del(key_t key)
{
    return this->skip_list->del(key);
}

/**
 * @brief 
 * 重置memtable
 * 包括重置过滤器以及跳表
 * 
 */
void MemTable::reset()
{
    this->bloom_filter->reset();
    this->skip_list->reset();
}