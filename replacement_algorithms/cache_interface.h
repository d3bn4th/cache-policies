#ifndef CACHE_INTERFACE_H
#define CACHE_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CACHE_SIZE 100

// Generic cache interface
typedef struct Cache Cache;

// Function declarations for LRU cache
Cache* create_lru_cache(int capacity);
void destroy_lru_cache(Cache* cache);
int get_lru(Cache* cache, int key);
void put_lru(Cache* cache, int key, int value);
void print_lru_cache_contents(Cache* cache, const char* message);

// Function declarations for LFU cache
Cache* create_lfu_cache(int capacity);
void destroy_lfu_cache(Cache* cache);
int get_lfu(Cache* cache, int key);
void put_lfu(Cache* cache, int key, int value);
void print_lfu_cache_contents(Cache* cache, const char* message);

// Function declarations for FIFO cache
Cache* create_fifo_cache(int capacity);
void destroy_fifo_cache(Cache* cache);
int get_fifo(Cache* cache, int key);
void put_fifo(Cache* cache, int key, int value);
void print_fifo_cache_contents(Cache* cache, const char* message);

// Function declarations for Random cache
Cache* create_random_cache(int capacity);
void destroy_random_cache(Cache* cache);
int get_random(Cache* cache, int key);
void put_random(Cache* cache, int key, int value);
void print_random_cache_contents(Cache* cache, const char* message);

#endif // CACHE_INTERFACE_H 