#ifndef CACHESYSTEM_CONSISTENT_H
#define CACHESYSTEM_CONSISTENT_H

#include <iostream>
#include <string>
using namespace std;

class ConsistentHash{
public:
    uint32_t hash(string key);
};


#endif //CACHESYSTEM_CONSISTENT_H
