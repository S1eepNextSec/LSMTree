#ifndef MEMTABLE_H
#define MEMTABLE_H
#include "skiplist.h"
#include "bloomfilter.h"
#include "utils.h"
#include <sstream>
#include <fstream>

class MemTable {
    using key_t = uint64_t;
    using val_t = std::string;
    using offset_t = uint64_t;
    using vlen_t = uint32_t;

private:
    std::string SSTABLE_SUFFIX = ".sst";
    int HEADER_BYTE = 32;
    int BLOOM_FILTER_BYTE = 8192;
    int TRIPLE_BYTE = 20;//8 + 8 + 4

private:
    BloomFilter *bloom_filter;
    skiplist::SkipList *skip_list;

private:
    uint64_t time_stamp; // 当前的时间戳
    char sstable_buffer[20000]={0x0};//用于生成sstable_buffer的内存缓冲区

private:
    //生成SSTable的头
    void generate_header(char *, uint64_t, uint64_t, key_t, key_t);

    // 生成SSTable的过滤器
    void generate_bloomfilter(char *);

    // 生成SSTable的三元组
    void generate_triple(char *,key_t,offset_t,vlen_t);

public:
    // 生成SSTable文件
    void createSSTable(const std::string &,offset_t offset_start);
    std::string naive_createSSTable();
    
    // 插入操作
    bool put(key_t, const val_t&);

    //重置操作
    void reset();

    //删除操作
    bool del(key_t);

public:
    MemTable(int TIME_STAMP);
    ~MemTable();

};

#endif // MEMTABLE_H