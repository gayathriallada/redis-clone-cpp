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

    cache.debugRemoveTail();
    std::cout << "After removeTail() (removes C): ";
    cache.debugPrint();   // expect: B <-> A

    cache.debugRemoveTail();
    std::cout << "After removeTail() again (removes A): ";
    cache.debugPrint();   // expect: B

    cache.debugRemoveTail();
    std::cout << "After removeTail() again (removes B, now empty): ";
    cache.debugPrint();   // expect: (nothing)

    return 0;
}
