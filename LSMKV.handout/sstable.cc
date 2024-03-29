#include "sstable.h"

SSTableController::SSTableController(int TIMESTAMP)
{
    time_stamp = TIMESTAMP;
}

SSTableController::~SSTableController()
{

}

void SSTableController::generate_header(char *start, uint64_t time_stamp, uint64_t key_val_count, key_t min, key_t max)
{
    *((uint64_t *)start) = time_stamp;
    start += 64;//time_stamp -> 64 bit

    *((uint64_t *)start) = key_val_count;
    start += 64; // key_val_count -> 64 bit

    *((uint64_t *)start) = min;
    start += 64; // min_key -> 64 bit

    *((uint64_t *)start) = max;
}

void SSTableController::generate_bloomfilter(char *start)
{
    std::memcpy(start, bloom_filter.getFilterString(), BLOOM_FILTER_BYTE);
}

void SSTableController::generate_triple(char *start, key_t key, offset_t offset, vlen_t vlen) 
{
    *((uint64_t *)start) = key;
    start += 64; // key -> 64 bit

    *((uint64_t *)start) = offset;
    start += 64; // offset -> 64 bit

    *((uint32_t *)start) = vlen;//vlen -> 32 bit
}

void SSTableController::createSSTable(const std::string &)
{

}

std::string SSTableController::naive_createSSTable()
{
    //优化?
    memset(sstable_buffer, 0x0, 20000);
}


