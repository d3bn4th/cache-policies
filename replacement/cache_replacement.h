#ifndef CACHE_REPLACEMENT_H
#define CACHE_REPLACEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CACHE_SIZE 100

// Cache entry structure
typedef struct CacheEntry {
    int key;
    int value;
    int frequency;      // For LFU
    int last_used;      // For LRU
    int time_added;     // For FIFO
} CacheEntry;

// Cache structure
typedef struct Cache {
    CacheEntry* entries;
    int size;
    int capacity;
    int current_time;
    int (*replacement_policy)(struct Cache*);  // Function pointer for replacement policy
} Cache;

// Function declarations
Cache* create_cache(int capacity);
void destroy_cache(Cache* cache);
int get(Cache* cache, int key);
void put(Cache* cache, int key, int value);

// Replacement policy functions
int lru_policy(Cache* cache);
int lfu_policy(Cache* cache);
int fifo_policy(Cache* cache);
int random_policy(Cache* cache);

// Utility functions
void print_cache_contents(Cache* cache, const char* message);
void test_cache(Cache* cache, const char* policy_name);
void run_interactive_mode();
void display_menu();

#endif // CACHE_REPLACEMENT_H 