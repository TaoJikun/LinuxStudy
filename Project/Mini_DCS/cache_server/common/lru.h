//
// Created by admin on 2022/5/13.
//

#ifndef DEMO_LRU_H
#define DEMO_LRU_H

#include <unordered_map>
#include <string>

struct CacheNode{
    std::string key;
    std::string val;
    CacheNode* pre,*next;
    CacheNode(std::string k,std::string v):key(k),val(v),pre(nullptr),next(nullptr){}
};

class LruCache{
public:
    LruCache(int capacity):size(capacity),head(nullptr),tail(nullptr){}

    void set(std::string key,std::string value){
        std::unordered_map<std::string,CacheNode*>::iterator it = mp.find(key);
        //找到了对应的map
        if(it!=mp.end()){
            //设置这个节点
            CacheNode* node = it->second;
            node->val = value;
            //删除节点
            remove(node);
            //将节点设置为头节点
            setHead(node);
        }else{
            CacheNode* node = new CacheNode(key,value);
            if(mp.size()>=size){
                std::unordered_map<std::string,CacheNode*>::iterator it = mp.find(tail->key);
                remove(tail);
                mp.erase(it);
            }
            setHead(node);
            mp[key] = node;
        }
    }

    std::string get(std::string key){
        std::unordered_map<std::string,CacheNode*>::iterator it = mp.find(key);
        if(it!=mp.end()){
            CacheNode* node = it->second;
            remove(node);
            setHead(node);
            return node->val;
        }else{
            return "";
        }
    }

    void remove(CacheNode* node){
        if(node->pre!=nullptr){
            node->pre->next = node->next;
        }else{
            head = node->next;
        }

        if(node->next!=nullptr){
            node->next->pre = node->pre;
        }else{
            tail = node->pre;
        }
    }

    void setHead(CacheNode* node){
        node->next = head;
        node->pre = nullptr;
        if(head!=nullptr){
            head->pre = node;
        }
        head = node;
        if(tail==nullptr){
            tail = head;
        }
    }

private:
    int size;
    CacheNode* head, *tail;
    std::unordered_map<std::string,CacheNode*> mp;
};


#endif //DEMO_LRU_H
