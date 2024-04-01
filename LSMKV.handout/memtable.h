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

    using magic_t = uint8_t;
    using check_t = uint16_t;

private:
    std::string SSTABLE_SUFFIX = ".sst";

    const int HEADER_BYTE = 32;
    const int BLOOM_FILTER_BYTE = 8192;
    const int TRIPLE_BYTE = 20;//8 + 8 + 4
    
    const int VLOG_MAGIC_BYTE = 1;
    const magic_t MAGIC_SIGN = 0xff;

    const int VLOG_CHECK_BYTE = 2;
    const int VLOG_KEY_BYTE = 8;
    const int VLOG_VLEN_BYTE = 4;

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

    void generate_vlog_entry(std::string &,key_t,vlen_t,const val_t&);

public:
    // 生成SSTable文件
    void createSSTable(const std::string &,offset_t offset_start,std::string &vlog_entry);
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