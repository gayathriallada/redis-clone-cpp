#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <string>
#include <ctime>

class LRUCache {
private:
    struct Node {
        std::string key;
        std::string value;
        Node* prev;
        Node* next;
        time_t expiry;
        Node(std::string k, std::string v) : key(k), value(v), prev(nullptr), next(nullptr), expiry(0) {}
    };

    int capacity;
    std::unordered_map<std::string, Node*> map;
    Node* head;
    Node* tail;

    void moveToFront(Node* node) {
        if (node == head) return;

        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == tail) tail = node->prev;

        node->prev = nullptr;
        node->next = head;
        if (head) head->prev = node;
        head = node;
        if (!tail) tail = node;
    }

    void removeTail() {
        if (!tail) return;

        map.erase(tail->key);

        Node* toDelete = tail;

        if (tail->prev) {
            tail = tail->prev;
            tail->next = nullptr;
        } else {
            head = nullptr;
            tail = nullptr;
        }

        delete toDelete;
    }

    bool isExpired(Node* node) {
        if (node->expiry == 0) return false;
        return time(nullptr) >= node->expiry;
    }

public:
    LRUCache(int cap) : capacity(cap), head(nullptr), tail(nullptr) {}

    void put(const std::string& key, const std::string& value, time_t expiry = 0) {
        if (map.count(key)) {
            Node* node = map[key];
            node->value = value;
            node->expiry = expiry;
            moveToFront(node);
            return;
        }

        if ((int)map.size() >= capacity) {
            removeTail();
        }

        Node* node = new Node(key, value);
        node->expiry = expiry;
        map[key] = node;
        moveToFront(node);
    }

    bool get(const std::string& key, std::string& value) {
        if (!map.count(key)) return false;
        Node* node = map[key];

        if (isExpired(node)) {
            remove(key);
            return false;
        }

        moveToFront(node);
        value = node->value;
        return true;
    }

    bool remove(const std::string& key) {
        if (!map.count(key)) return false;
        Node* node = map[key];

        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == head) head = node->next;
        if (node == tail) tail = node->prev;

        map.erase(key);
        delete node;
        return true;
    }

    void sweepExpired() {
        Node* current = tail;
        while (current) {
            Node* prevNode = current->prev;
            if (isExpired(current)) {
                remove(current->key);
            }
            current = prevNode;
        }
    }
};

#endif
