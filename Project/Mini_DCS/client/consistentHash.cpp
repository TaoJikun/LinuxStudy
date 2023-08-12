//
// Created by admin on 2022/5/18.
//

#include "consistentHash.h"


//32位的 Fowler-Noll-Vo 哈希算法
uint32_t ConsistentHash::hash(string key){
    const int p = 16777619;
    uint32_t hash = 2166136261;

    for (int idx = 0; idx < key.size(); ++idx) {
        hash = (hash ^ key[idx]) * p;
    }
    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;

    if (hash < 0) {
        hash = -hash;
    }

    return hash;
}
