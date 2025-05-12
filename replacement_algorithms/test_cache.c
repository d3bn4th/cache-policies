#include "cache_interface.h"
#include "lru_cache.h"
#include "lfu_cache.h"
#include "fifo_cache.h"
#include "random_cache.h"

void test_cache(Cache* cache, const char* name) {
    printf("Testing %s Cache:\n", name);
    put(cache, 1, 100);
    put(cache, 2, 200);
    put(cache, 3, 300);
    printf("Get 1: %d\n", get(cache, 1)); // Should return 100
    put(cache, 4, 400); // May evict an entry based on the policy
    print_cache_contents(cache, "Current Cache Contents");
    destroy_cache(cache);
}

int main() {
    Cache* lru_cache = create_cache(MAX_CACHE_SIZE);
    Cache* lfu_cache = create_cache(MAX_CACHE_SIZE);
    Cache* fifo_cache = create_cache(MAX_CACHE_SIZE);
    Cache* random_cache = create_cache(MAX_CACHE_SIZE);

    test_cache(lru_cache, "LRU");
    test_cache(lfu_cache, "LFU");
    test_cache(fifo_cache, "FIFO");
    test_cache(random_cache, "Random");

    return 0;
}