#ifndef CACHE_WRITE_H
#define CACHE_WRITE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CACHE_SIZE 100
#define MEMORY_SIZE 1000

// Memory structure declaration
typedef struct {
    int data[MEMORY_SIZE];
    int initialized[MEMORY_SIZE];  // Tracks which memory locations have been written to
} Memory;

// Cache entry structure
typedef struct CacheEntry {
    int key;
    int value;
    int dirty;          // For write-back
    int valid;          // Valid bit
    time_t last_modified;  // For write-through timing
} CacheEntry;

// Cache structure
typedef struct Cache {
    CacheEntry* entries;
    int size;
    int capacity;
    int current_time;
    int (*write_policy)(struct Cache*, int, int);  // Function pointer for write policy
} Cache;

// Memory operations
void init_memory(void);
int memory_read(int address);
void memory_write(int address, int value);

// Cache operations
Cache* create_cache(int capacity);
void destroy_cache(Cache* cache);
int read(Cache* cache, int key);
int write(Cache* cache, int key, int value);

// Write policy functions
int write_through(Cache* cache, int key, int value);
int write_back(Cache* cache, int key, int value);
int write_around(Cache* cache, int key, int value);
int write_back_no_allocate(Cache* cache, int key, int value);

// Utility functions
void print_cache_contents(Cache* cache, const char* message);
void print_memory_contents(int start_addr, int end_addr, const char* message);
void test_cache(Cache* cache, const char* policy_name);
void run_interactive_mode(Cache* cache);
void display_menu(void);

// Global memory variable declaration
extern Memory main_memory;

#endif // CACHE_WRITE_H 