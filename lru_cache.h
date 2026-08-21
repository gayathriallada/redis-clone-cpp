#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <string>
#include <iostream>
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
    void moveToFront(Node* node) {
        if (node == head) return;

        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == tail) tail = node->prev;

        node->prev = nullptr;
        node->next = head;
        if (head) head->prev = node;
        head = node;
    }
    void removeTail() {
        if (!tail) return;
        Node* nodeToRemove = tail;

        if (tail == head) {
            head = nullptr;
            tail = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }

        map.erase(nodeToRemove->key);
        delete nodeToRemove;
    }
public:
    LRUCache(int cap) : capacity(cap), head(nullptr), tail(nullptr) {}
 void debugPrint() {
        Node* cur = head;
        while (cur) {
            std::cout << cur->key;
            if (cur->next) std::cout << " <-> ";
            cur = cur->next;
        }
        std::cout << "\n";
    }

    // TEMPORARY — for testing moveToFront in isolation only.
    // Delete this once put() exists and can build lists the real way.
    void debugBuildThreeNodeList() {
        Node* a = new Node("A", "vA");
        Node* b = new Node("B", "vB");
        Node* c = new Node("C", "vC");

        a->next = b; b->prev = a;
        b->next = c; c->prev = b;

        head = a;
        tail = c;

        map["A"] = a;
        map["B"] = b;
        map["C"] = c;
    }

    // TEMPORARY — exposes moveToFront for the test file to call directly.
    // Delete this once get()/put() call moveToFront internally instead.
    void debugMoveToFront(const std::string& key) {
        moveToFront(map[key]);
    }
    // TEMPORARY — exposes removeTail for the test file to call directly.
    // Delete this once put() calls removeTail internally when over capacity.
    void debugRemoveTail() {
        removeTail();
    }
};

#endif

