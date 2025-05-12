#include "lfu_cache.h"
#include <string.h>

#define HASH_SIZE 1000

// Node structure for doubly linked list with frequency
typedef struct LFUNode {
    int key;
    int value;
    int frequency;
    struct LFUNode* prev;
    struct LFUNode* next;
} LFUNode;

// Hash entry structure
typedef struct HashEntry {
    int key;
    LFUNode* node;
    struct HashEntry* next;
} HashEntry;

// Cache structure
struct Cache {
    LFUNode* head;
    LFUNode* tail;
    HashEntry** hash_table;
    int size;
    int capacity;
};

// Hash function
static unsigned int hash(int key) {
    return (unsigned int)key % HASH_SIZE;
}

// Create a new LFU node
static LFUNode* create_node(int key, int value) {
    LFUNode* node = (LFUNode*)malloc(sizeof(LFUNode));
    if (node) {
        node->key = key;
        node->value = value;
        node->frequency = 1;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

// Create a new hash entry
static HashEntry* create_hash_entry(int key, LFUNode* node) {
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (entry) {
        entry->key = key;
        entry->node = node;
        entry->next = NULL;
    }
    return entry;
}

// Add node to list
static void add_node(Cache* cache, LFUNode* node) {
    if (!cache->head) {
        cache->head = node;
        cache->tail = node;
        return;
    }
    
    // Add to end of list
    node->prev = cache->tail;
    cache->tail->next = node;
    cache->tail = node;
}

// Remove node from list
static void remove_node(Cache* cache, LFUNode* node) {
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
static void add_to_hash(Cache* cache, int key, LFUNode* node) {
    unsigned int h = hash(key);
    HashEntry* entry = create_hash_entry(key, node);
    entry->next = cache->hash_table[h];
    cache->hash_table[h] = entry;
}

// Find least frequently used node
static LFUNode* find_lfu_node(Cache* cache) {
    if (!cache->head) {
        return NULL;
    }

    LFUNode* lfu = cache->head;
    LFUNode* current = cache->head->next;

    while (current) {
        if (current->frequency < lfu->frequency) {
            lfu = current;
        }
        current = current->next;
    }

    return lfu;
}

// Create a new cache
Cache* create_lfu_cache(int capacity) {
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

    return cache;
}

// Destroy the cache
void destroy_lfu_cache(Cache* cache) {
    if (!cache) {
        return;
    }

    // Free all nodes in the linked list
    LFUNode* current = cache->head;
    while (current) {
        LFUNode* next = current->next;
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
int get_lfu(Cache* cache, int key) {
    if (!cache) {
        return -1;
    }

    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            entry->node->frequency++;
            return entry->node->value;
        }
        entry = entry->next;
    }
    
    return -1;  // Key not found
}

// Put value in cache
void put_lfu(Cache* cache, int key, int value) {
    if (!cache) {
        return;
    }

    // Check if key exists
    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            entry->node->value = value;
            entry->node->frequency++;
            return;
        }
        entry = entry->next;
    }

    // Create new node
    LFUNode* new_node = create_node(key, value);
    if (!new_node) {
        return;
    }

    // If cache is full, remove least frequently used
    if (cache->size >= cache->capacity) {
        LFUNode* lfu = find_lfu_node(cache);
        if (lfu) {
            remove_node(cache, lfu);
            remove_from_hash(cache, lfu->key);
            free(lfu);
            cache->size--;
        }
    }

    // Add new node
    add_node(cache, new_node);
    add_to_hash(cache, key, new_node);
    cache->size++;
}

// Print cache contents
void print_lfu_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Cache contents (Ordered by Insertion):\n");
    printf("------------------------------------------------\n");
    printf("Key\tValue\tFrequency\n");
    printf("------------------------------------------------\n");
    
    LFUNode* current = cache->head;
    while (current) {
        printf("%d\t%d\t%d\n", 
               current->key, 
               current->value, 
               current->frequency);
        current = current->next;
    }
    printf("------------------------------------------------\n");
    printf("Cache size: %d/%d\n", cache->size, cache->capacity);
} 