#ifndef CACHE_REPLACEMENT_H
#define CACHE_REPLACEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CACHE_SIZE 100
#define HASH_SIZE 1000  // Size of hash table, should be larger than cache size

// Node structure for doubly linked list
typedef struct LRUNode {
    int key;
    int value;
    struct LRUNode* prev;
    struct LRUNode* next;
    int frequency;      // For LFU policy
    int time_added;     // For FIFO policy
} LRUNode;

// Hash entry structure
typedef struct HashEntry {
    int key;
    LRUNode* node;
    struct HashEntry* next;  // For handling collisions
} HashEntry;

// Cache structure
typedef struct Cache {
    LRUNode* head;      // Most recently used
    LRUNode* tail;      // Least recently used
    HashEntry** hash_table;
    int size;
    int capacity;
    int current_time;   // For tracking insertion order
    int (*replacement_policy)(struct Cache*);
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