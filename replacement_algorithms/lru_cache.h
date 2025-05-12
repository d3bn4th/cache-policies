#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include "cache_interface.h"

Cache* create_lru_cache(int capacity);
void destroy_lru_cache(Cache* cache);
int get_lru(Cache* cache, int key);
void put_lru(Cache* cache, int key, int value);
void print_lru_cache_contents(Cache* cache, const char* message);

// LRU specific declarations can be added here if needed

#endif // LRU_CACHE_H 