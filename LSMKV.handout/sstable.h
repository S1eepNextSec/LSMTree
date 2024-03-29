#ifndef SSTABLE_H
#define SSTABLE_H
#include "skiplist.h"
#include "bloomfilter.h"
#include "utils.h"
class SSTableController {
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
    BloomFilter bloom_filter;
    skiplist::SkipList skip_list;

private:
    uint64_t time_stamp; // 当前的时间戳
    char sstable_buffer[20000]={0x0};//用于生成sstable_buffer的内存缓冲区

private:
    void generate_header(char *, uint64_t, uint64_t, key_t, key_t);
    void generate_bloomfilter(char *);
    void generate_triple(char *,key_t,offset_t,vlen_t);
    void createSSTable(const std::string &);
    std::string naive_createSSTable();

public:
    SSTableController(int TIME_STAMP);
    ~SSTableController();

};

#endif // SSTABLE_H