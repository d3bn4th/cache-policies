#ifndef FIFO_CACHE_H
#define FIFO_CACHE_H

#include "cache_interface.h"

Cache* create_fifo_cache(int capacity);
void destroy_fifo_cache(Cache* cache);
int get_fifo(Cache* cache, int key);
void put_fifo(Cache* cache, int key, int value);
void print_fifo_cache_contents(Cache* cache, const char* message);

// FIFO specific declarations can be added here if needed

#endif // FIFO_CACHE_H 