#include "skiplist.h"
#include <optional>
#include <cmath>
#include <assert.h>
#include <random>
namespace skiplist {

//初始化skiplist
//给定生长因子p
SkipList::SkipList(double P, int element_count) : p(P), n(element_count)
{
    rng.seed(rd());
    // 定义出maxLevel层数上限
    maxLevel = log(n) / log(1 / p);

    //创建头节点
    head = new skipNode(min_key,"",maxLevel);
    //创建尾节点
    skipNode *tail = new skipNode(max_key, "", maxLevel);
    
    for (int i = 1; i <= maxLevel;i++){
        head->forward[i] = tail;//头节点指向尾节点
        tail->forward[i] = NULL;//尾节点设空
    }

    level = 1;//设置当前总的高度是1
}

SkipList::~SkipList()
{
    skipNode *temp = head;
    while (head != NULL){
        temp = temp->forward[1];
        delete head;
        head = temp;
    }
}

void SkipList::put(key_t new_key, const value_t &new_val)
{
    skipNode *update[maxLevel+1];//update指针数组记录某一层发生下跳的节点

    skipNode *x = this->head;//指针x用来寻找节点，从头节点开始

    for (int i = level; i >= 1;i--){//i为当前所在层数 从最高层开始寻找
        while (new_key > x->forward[i]->key)//向右跳直到遇到一个比要插入的键大的键
            x = x->forward[i];
        assert(new_key <= x->forward[i]->key && new_key > x->key);
        update[i] = x;
    }

    x = x->forward[1];

    //如果该键已经存在 就覆盖原本的值
    if (x->key == new_key){
        x->val = new_val;
        return;
    } 

    //该键不存在，就新建一个节点
    //随机生成新节点的层数
    int level_new_node = randomLevel();

    //新建节点
    skipNode *new_node = new skipNode(new_key, new_val, level_new_node);
    
    //如果新节点的层数大于当前最高层数
    if (level_new_node > level){
        //设置超出部分的update元素为头节点
        for (int i = level + 1; i <= level_new_node;i++){
            update[i] = head;
        }
        level = level_new_node;
    }

    for (int i = 1; i <= level_new_node; i++) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }
}

value_t SkipList::get(key_t key) const {
    skipNode *x = this->head;
    for (int i = level; i >= 1;i--){
        while(key>x->forward[i]->key)
            x = x->forward[i];
        assert(key <= x->forward[i]->key && key > x->key);
    }
    x = x->forward[1];
    if (x->key == key){
        return x->val;
    }
    else {
        return "";
    }
}

int SkipList::randomLevel()
{
    int tempLevel = 1;

    std::uniform_real_distribution<> rand_num(0.0, 1.0);
    while (rand_num(rng)< p && tempLevel < maxLevel) {
        tempLevel++;
    }
    return tempLevel;
}

} // namespace skiplist