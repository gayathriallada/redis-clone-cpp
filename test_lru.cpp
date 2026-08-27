#include "lru_cache.h"
#include <iostream>

int main() {
    LRUCache cache(5);
    cache.put("name", "gayathri");
    cache.put("city", "hyderabad");

    cache.save("test_snapshot.bin");
    std::cout << "Saved to test_snapshot.bin\n";

    return 0;
}
