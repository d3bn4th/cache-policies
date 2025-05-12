#ifndef LFU_CACHE_H
#define LFU_CACHE_H

#include "cache_interface.h"

Cache* create_lfu_cache(int capacity);
void destroy_lfu_cache(Cache* cache);
int get_lfu(Cache* cache, int key);
void put_lfu(Cache* cache, int key, int value);
void print_lfu_cache_contents(Cache* cache, const char* message);

// LFU specific declarations can be added here if needed

#endif // LFU_CACHE_H 