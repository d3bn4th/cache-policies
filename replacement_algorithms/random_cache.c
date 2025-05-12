#include "random_cache.h"
#include <string.h>
#include <time.h>

#define HASH_SIZE 1000

// Node structure for linked list
typedef struct Node {
    int key;
    int value;
    struct Node* prev;
    struct Node* next;
} Node;

// Hash entry structure
typedef struct HashEntry {
    int key;
    Node* node;
    struct HashEntry* next;
} HashEntry;

// Cache structure
struct Cache {
    Node* head;
    Node* tail;
    HashEntry** hash_table;
    int size;
    int capacity;
};

// Hash function
static unsigned int hash(int key) {
    return (unsigned int)key % HASH_SIZE;
}

// Create a new node
static Node* create_node(int key, int value) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->key = key;
        node->value = value;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

// Create a new hash entry
static HashEntry* create_hash_entry(int key, Node* node) {
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (entry) {
        entry->key = key;
        entry->node = node;
        entry->next = NULL;
    }
    return entry;
}

// Add node to list
static void add_node(Cache* cache, Node* node) {
    if (!cache->tail) {
        cache->head = node;
        cache->tail = node;
    } else {
        node->prev = cache->tail;
        cache->tail->next = node;
        cache->tail = node;
    }
}

// Remove node from list
static void remove_node(Cache* cache, Node* node) {
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
static void add_to_hash(Cache* cache, int key, Node* node) {
    unsigned int h = hash(key);
    HashEntry* entry = create_hash_entry(key, node);
    entry->next = cache->hash_table[h];
    cache->hash_table[h] = entry;
}

// Get random node from cache
static Node* get_random_node(Cache* cache) {
    if (!cache->head) {
        return NULL;
    }

    int random_pos = rand() % cache->size;
    Node* current = cache->head;
    
    while (random_pos > 0 && current) {
        current = current->next;
        random_pos--;
    }

    return current;
}

// Create a new cache
Cache* create_random_cache(int capacity) {
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

    // Initialize random seed
    srand(time(NULL));

    return cache;
}

// Destroy the cache
void destroy_random_cache(Cache* cache) {
    if (!cache) {
        return;
    }

    // Free all nodes in the list
    Node* current = cache->head;
    while (current) {
        Node* next = current->next;
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
int get_random(Cache* cache, int key) {
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
void put_random(Cache* cache, int key, int value) {
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
    Node* new_node = create_node(key, value);
    if (!new_node) {
        return;
    }

    // If cache is full, remove random entry
    if (cache->size >= cache->capacity) {
        Node* random_node = get_random_node(cache);
        if (random_node) {
            remove_node(cache, random_node);
            remove_from_hash(cache, random_node->key);
            free(random_node);
            cache->size--;
        }
    }

    // Add new node
    add_node(cache, new_node);
    add_to_hash(cache, key, new_node);
    cache->size++;
}

// Print cache contents
void print_random_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Cache contents:\n");
    printf("------------------------------------------------\n");
    printf("Key\tValue\n");
    printf("------------------------------------------------\n");
    
    Node* current = cache->head;
    while (current) {
        printf("%d\t%d\n", current->key, current->value);
        current = current->next;
    }
    printf("------------------------------------------------\n");
    printf("Cache size: %d/%d\n", cache->size, cache->capacity);
} 