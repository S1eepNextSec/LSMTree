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
    start += 64;//time_stamp -> 64 bit

    *((uint64_t *)start) = key_val_count;
    start += 64; // key_val_count -> 64 bit

    *((uint64_t *)start) = min;
    start += 64; // min_key -> 64 bit

    *((uint64_t *)start) = max;
}

void MemTable::generate_bloomfilter(char *start)
{
    std::memcpy(start, bloom_filter->getFilterString(), BLOOM_FILTER_BYTE);
}

void MemTable::generate_triple(char *start, key_t key, offset_t offset, vlen_t vlen) 
{
    *((uint64_t *)start) = key;
    start += 64; // key -> 64 bit

    *((uint64_t *)start) = offset;
    start += 64; // offset -> 64 bit

    *((uint32_t *)start) = vlen;//vlen -> 32 bit
}


/**
 * @param path 要创建新的SSTable的位置
 * @param offset_start 在vlog中这块数据的起始偏移位置
*/
void MemTable::createSSTable(const std::string &path,offset_t offset_start)
{
    // 优化?
    //清空用来写sstable中内容的字符串缓冲区
    memset(sstable_buffer, 0x0, 20000);
    
    char *head_start = sstable_buffer;
    char *bloom_filter_start = head_start + HEADER_BYTE;
    char *triple_start = bloom_filter_start + BLOOM_FILTER_BYTE;

    skiplist::skipNode *skip_node_ptr = this->skip_list->getHeader()->forward[1];

    key_t max_key;
    key_t min_key = skip_node_ptr->key;
    key_t key;
    offset_t offset = offset_start;
    vlen_t vlen;
    int count = 0;
    // 写三元组部分
    while (skip_node_ptr->forward[1]!=NULL){
        
        count ++;//数据量++

        key = skip_node_ptr->key;          // 获取键值
        vlen = skip_node_ptr->val.length();//获取字符串长度

        generate_triple(triple_start, key, offset, vlen);//写入三元组

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
        file << this->sstable_buffer;
        file.close();
    }
}


std::string MemTable::naive_createSSTable()
{
}


bool MemTable::put(key_t key,const val_t &val)
{
    //如果再插入一个就超额了 就需要创建sstable并写入vlog中
    if (this->skip_list->isFull())
        return 0;

    this->skip_list->put(key, val);
    this->bloom_filter->insert(key);

    return 1;
}