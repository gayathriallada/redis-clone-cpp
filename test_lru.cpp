#include "lru_cache.h"
#include <iostream>

int main() {
    LRUCache cache(3);

    cache.debugBuildThreeNodeList();
    std::cout << "Before: ";
    cache.debugPrint();   // expect: A <-> B <-> C

    cache.debugMoveToFront("B");
    std::cout << "After moveToFront(B): ";
    cache.debugPrint();   // expect: B <-> A <-> C

    return 0;
}
