#ifndef LRU_HPP
#define LRU_HPP
#include <unordered_map>
#include <cstring>
struct DLinkedNode {
    std::string key, value;
    DLinkedNode* prev;
    DLinkedNode* next;
    DLinkedNode(): key(20,' '), value(200,' '), prev(nullptr), next(nullptr) {}
    DLinkedNode(std::string _key, std::string _value): key(_key), value(_value), prev(nullptr), next(nullptr) {}
};
class LRUCache {
private:
    int capacity;
    void addToHead(DLinkedNode* node);
    void removeNode(DLinkedNode* node);
    void moveToHead(DLinkedNode* node);
    DLinkedNode* removeTail();
public:
    std::unordered_map<std::string, DLinkedNode*> cache;
    std::vector<std::string> key_vec;
    DLinkedNode* head;
    DLinkedNode* tail;
    int size;
    LRUCache(int _capacity);
    std::string get(std::string key, bool flag);
    void put(std::string key, std::string value);
};

LRUCache::LRUCache(int _capacity): capacity(_capacity), size(0) {
    head = new DLinkedNode();
    tail = new DLinkedNode();
    head->next = tail;
    tail->prev = head;
}
std::string LRUCache::get(std::string key, bool flag=true) {
    if (!cache.count(key)) {
        return "";
    }
    DLinkedNode* node = cache[key];
    if(flag)
        moveToHead(node);
        
    return node->value;
}
void LRUCache::put(std::string key, std::string value) {
    if (!cache.count(key)) {
        DLinkedNode* node = new DLinkedNode(key, value);
        cache[key] = node;
        addToHead(node);
        ++size;
        if (size > capacity) {
            DLinkedNode* removed = removeTail();
            cache.erase(removed->key);
            delete removed;
            --size;
        }
    }
    else {
        DLinkedNode* node = cache[key];
        node->value = value;
        moveToHead(node);
    }
}
void LRUCache::addToHead(DLinkedNode* node) {
    node->prev = head;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
}
void LRUCache::removeNode(DLinkedNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}
void LRUCache::moveToHead(DLinkedNode* node) {
    removeNode(node);
    addToHead(node);
}
DLinkedNode* LRUCache::removeTail() {
    DLinkedNode* node = tail->prev;
    removeNode(node);
    return node;
}
#endif /* LRU_HPP */
