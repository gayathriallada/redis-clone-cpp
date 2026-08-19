#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <string>

class LRUCache {
private:
    struct Node {
        std::string key;
        std::string value;
        Node* prev;
        Node* next;
        Node(std::string k, std::string v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };

    int capacity;
    std::unordered_map<std::string, Node*> map;
    Node* head;
    Node* tail;
public:
    LRUCache(int cap) : capacity(cap), head(nullptr), tail(nullptr) {}
};

#endif
