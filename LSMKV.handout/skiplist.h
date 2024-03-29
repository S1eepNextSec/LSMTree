#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <cstdint>
#include <limits>
#include <random>
// #include <optional>
// #include <vector>
#include <string>
#include <map>

namespace skiplist {
using key_t = uint64_t;
using value_t = std::string;

struct skipNode {
    key_t key;//节点的键
    value_t val;//节点的值
    skipNode **forward;//记录每层指向下一节点的指针数组
    int level;//当前节点的层数
    skipNode(const key_t &Key, const value_t &Val, const int &nodeLevel)
    {
        key = Key;
        val = Val;
        level = nodeLevel;
        
		forward = new skipNode *[nodeLevel + 1];//0 && 1 ~ level
    }

    ~skipNode()
	{
        delete forward;
    }
};

class SkipList
{
	// add something here
    private:
        key_t min_key = std::numeric_limits<skiplist::key_t>::min();
        key_t max_key = std::numeric_limits<skiplist::key_t>::max();

    private:
        double p;//生长因子
        int maxLevel;//整个跳表最大层数 由p和n决定
        int n;//数据数量
        skipNode* head;//头节点
        int level;//当前层数

        std::random_device rd;
        std::mt19937_64 rng;

    private:
        //随机生成高度
        int randomLevel();

    public:
        explicit SkipList(double p, int element_count);
        
        //插入
        void put(key_t key, const value_t &val);
        //查询
        value_t get(key_t key) const;
        //删除
        bool del(key_t key);

        //区间查找
        std::map<key_t,value_t> scan(key_t key1,key_t key2);

        ~SkipList();

        void printlist(){
            skipNode *x = head;
			while (x!=NULL){
                for (int i = 1; i <= x->level;i++){
                    printf("%d\t", x->key);
                }
                printf("\n");
                x = x->forward[1];
            }
        }
};

} // namespace skiplist

#endif // SKIPLIST_H