#ifndef RANDOM_CACHE_H
#define RANDOM_CACHE_H

#include "cache_interface.h"

Cache* create_random_cache(int capacity);
void destroy_random_cache(Cache* cache);
int get_random(Cache* cache, int key);
void put_random(Cache* cache, int key, int value);
void print_random_cache_contents(Cache* cache, const char* message);

// Random specific declarations can be added here if needed

#endif // RANDOM_CACHE_H 