#include "lru_cache.h"
#include <string.h>

#define HASH_SIZE 1000

// Node structure for doubly linked list
typedef struct LRUNode {
    int key;
    int value;
    struct LRUNode* prev;
    struct LRUNode* next;
} LRUNode;

// Hash entry structure
typedef struct HashEntry {
    int key;
    LRUNode* node;
    struct HashEntry* next;
} HashEntry;

// Cache structure
struct Cache {
    LRUNode* head;      // Most recently used
    LRUNode* tail;      // Least recently used
    HashEntry** hash_table;
    int size;
    int capacity;
};

// Hash function
static unsigned int hash(int key) {
    return (unsigned int)key % HASH_SIZE;
}

// Create a new LRU node
static LRUNode* create_node(int key, int value) {
    LRUNode* node = (LRUNode*)malloc(sizeof(LRUNode));
    if (node) {
        node->key = key;
        node->value = value;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

// Create a new hash entry
static HashEntry* create_hash_entry(int key, LRUNode* node) {
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (entry) {
        entry->key = key;
        entry->node = node;
        entry->next = NULL;
    }
    return entry;
}

// Add node to front of list (most recently used)
static void add_to_front(Cache* cache, LRUNode* node) {
    node->next = cache->head;
    node->prev = NULL;
    
    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;
    
    if (!cache->tail) {
        cache->tail = node;
    }
}

// Remove node from list
static void remove_node(Cache* cache, LRUNode* node) {
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

// Move node to front (most recently used)
static void move_to_front(Cache* cache, LRUNode* node) {
    remove_node(cache, node);
    add_to_front(cache, node);
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
static void add_to_hash(Cache* cache, int key, LRUNode* node) {
    unsigned int h = hash(key);
    HashEntry* entry = create_hash_entry(key, node);
    entry->next = cache->hash_table[h];
    cache->hash_table[h] = entry;
}

// Create a new cache
Cache* create_lru_cache(int capacity) {
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
void destroy_lru_cache(Cache* cache) {
    if (!cache) {
        return;
    }

    // Free all nodes in the linked list
    LRUNode* current = cache->head;
    while (current) {
        LRUNode* next = current->next;
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
int get_lru(Cache* cache, int key) {
    if (!cache) {
        return -1;
    }

    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            move_to_front(cache, entry->node);
            return entry->node->value;
        }
        entry = entry->next;
    }
    
    return -1;  // Key not found
}

// Put value in cache
void put_lru(Cache* cache, int key, int value) {
    if (!cache) {
        return;
    }

    // Check if key exists
    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            entry->node->value = value;
            move_to_front(cache, entry->node);
            return;
        }
        entry = entry->next;
    }

    // Create new node
    LRUNode* new_node = create_node(key, value);
    if (!new_node) {
        return;
    }

    // If cache is full, remove least recently used
    if (cache->size >= cache->capacity) {
        LRUNode* lru = cache->tail;
        remove_node(cache, lru);
        remove_from_hash(cache, lru->key);
        free(lru);
        cache->size--;
    }

    // Add new node
    add_to_front(cache, new_node);
    add_to_hash(cache, key, new_node);
    cache->size++;
}

// Print cache contents
void print_lru_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Cache contents (Most Recent → Least Recent):\n");
    printf("------------------------------------------------\n");
    printf("Key\tValue\n");
    printf("------------------------------------------------\n");
    
    LRUNode* current = cache->head;
    while (current) {
        printf("%d\t%d\n", current->key, current->value);
        current = current->next;
    }
    printf("------------------------------------------------\n");
    printf("Cache size: %d/%d\n", cache->size, cache->capacity);
} 