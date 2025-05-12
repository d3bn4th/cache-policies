#include "fifo_cache.h"
#include <string.h>

#define HASH_SIZE 1000

// Node structure for queue
typedef struct FIFONode {
    int key;
    int value;
    int time_added;
    struct FIFONode* prev;
    struct FIFONode* next;
} FIFONode;

// Hash entry structure
typedef struct HashEntry {
    int key;
    FIFONode* node;
    struct HashEntry* next;
} HashEntry;

// Cache structure
struct Cache {
    FIFONode* head;    // First in (oldest)
    FIFONode* tail;    // Last in (newest)
    HashEntry** hash_table;
    int size;
    int capacity;
    int current_time;
};

// Hash function
static unsigned int hash(int key) {
    return (unsigned int)key % HASH_SIZE;
}

// Create a new FIFO node
static FIFONode* create_node(int key, int value, Cache* cache) {
    FIFONode* node = (FIFONode*)malloc(sizeof(FIFONode));
    if (node) {
        node->key = key;
        node->value = value;
        node->time_added = cache->current_time++;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

// Create a new hash entry
static HashEntry* create_hash_entry(int key, FIFONode* node) {
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (entry) {
        entry->key = key;
        entry->node = node;
        entry->next = NULL;
    }
    return entry;
}

// Add node to end of queue (newest)
static void add_to_queue(Cache* cache, FIFONode* node) {
    if (!cache->tail) {
        cache->head = node;
        cache->tail = node;
    } else {
        node->prev = cache->tail;
        cache->tail->next = node;
        cache->tail = node;
    }
}

// Remove node from queue
static void remove_node(Cache* cache, FIFONode* node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        cache->head = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        cache->tail = node->prev;
    }
}

// Remove entry from hash table
static void remove_from_hash(Cache* cache, int key) {
    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    HashEntry* prev = NULL;

    while (entry) {
        if (entry->key == key) {
            if (prev) {
                prev->next = entry->next;
            } else {
                cache->hash_table[h] = entry->next;
            }
            free(entry);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

// Add entry to hash table
static void add_to_hash(Cache* cache, int key, FIFONode* node) {
    unsigned int h = hash(key);
    HashEntry* entry = create_hash_entry(key, node);
    entry->next = cache->hash_table[h];
    cache->hash_table[h] = entry;
}

// Create a new cache
Cache* create_fifo_cache(int capacity) {
    if (capacity <= 0 || capacity > MAX_CACHE_SIZE) {
        return NULL;
    }

    Cache* cache = (Cache*)malloc(sizeof(Cache));
    if (!cache) {
        return NULL;
    }

    cache->hash_table = (HashEntry**)calloc(HASH_SIZE, sizeof(HashEntry*));
    if (!cache->hash_table) {
        free(cache);
        return NULL;
    }

    cache->head = NULL;
    cache->tail = NULL;
    cache->size = 0;
    cache->capacity = capacity;
    cache->current_time = 0;

    return cache;
}

// Destroy the cache
void destroy_fifo_cache(Cache* cache) {
    if (!cache) {
        return;
    }

    // Free all nodes in the queue
    FIFONode* current = cache->head;
    while (current) {
        FIFONode* next = current->next;
        free(current);
        current = next;
    }

    // Free all hash entries
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry* entry = cache->hash_table[i];
        while (entry) {
            HashEntry* next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(cache->hash_table);
    free(cache);
}

// Get value from cache
int get_fifo(Cache* cache, int key) {
    if (!cache) {
        return -1;
    }

    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            return entry->node->value;
        }
        entry = entry->next;
    }
    
    return -1;  // Key not found
}

// Put value in cache
void put_fifo(Cache* cache, int key, int value) {
    if (!cache) {
        return;
    }

    // Check if key exists
    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            entry->node->value = value;
            return;
        }
        entry = entry->next;
    }

    // Create new node
    FIFONode* new_node = create_node(key, value, cache);
    if (!new_node) {
        return;
    }

    // If cache is full, remove oldest entry (from head)
    if (cache->size >= cache->capacity) {
        FIFONode* oldest = cache->head;
        remove_node(cache, oldest);
        remove_from_hash(cache, oldest->key);
        free(oldest);
        cache->size--;
    }

    // Add new node to end of queue
    add_to_queue(cache, new_node);
    add_to_hash(cache, key, new_node);
    cache->size++;
}

// Print cache contents
void print_fifo_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Cache contents (First In → Last In):\n");
    printf("------------------------------------------------\n");
    printf("Key\tValue\tTime Added\n");
    printf("------------------------------------------------\n");
    
    FIFONode* current = cache->head;
    while (current) {
        printf("%d\t%d\t%d\n", 
               current->key, 
               current->value, 
               current->time_added);
        current = current->next;
    }
    printf("------------------------------------------------\n");
    printf("Cache size: %d/%d\n", cache->size, cache->capacity);
} 