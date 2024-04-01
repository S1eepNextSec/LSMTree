#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H
#include "MurmurHash3.h"
#include <cstring>

class BloomFilter{
    private:
        int size;//过滤器的大小
        char *filter_array;//过滤器数组

    public:
        BloomFilter(int SIZE){
            size = SIZE;
            filter_array = new char[SIZE];//0 ~ SIZE-1
            std::memset(filter_array, 0x00, size * sizeof(char));
        }

        void insert(uint64_t key){
            unsigned int hash[4] = {0};
            MurmurHash3_x64_128(&key, sizeof(key), 1, hash);

            for (int i = 0; i < 4;i++)
                filter_array[hash[i] % size] = 0x01;
        }

        bool find(uint64_t key){
            unsigned int hash[4] = {0};
            MurmurHash3_x64_128(&key, sizeof(key), 1, hash);

            for (int i = 0; i < 4; i++){
                if (!filter_array[hash[i] % size])
                    return false;
            }
            return true;
        }

        void reset(){
            std::memset(filter_array, 0x00, size * sizeof(char));
        }

        char* getFilterString(){
            return filter_array;
        }

        ~BloomFilter(){
            delete[] filter_array;
        }
};
#endif // BLOOMFILTER_H