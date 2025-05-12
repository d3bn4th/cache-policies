#include "cache_replacement.h"
#include <string.h>

// Hash function
static unsigned int hash(int key) {
    return (unsigned int)key % HASH_SIZE;
}

// Create a new LRU node
static LRUNode* create_node(int key, int value, Cache* cache) {
    LRUNode* node = (LRUNode*)malloc(sizeof(LRUNode));
    if (node) {
        node->key = key;
        node->value = value;
        node->prev = NULL;
        node->next = NULL;
        node->frequency = 1;
        node->time_added = cache->current_time++;
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

// Create a new cache with specified capacity
Cache* create_cache(int capacity) {
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
    cache->replacement_policy = NULL;

    return cache;
}

// Destroy the cache and free memory
void destroy_cache(Cache* cache) {
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

// Get value for a key from cache
int get(Cache* cache, int key) {
    if (!cache) {
        return -1;
    }

    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            // Update frequency for LFU
            entry->node->frequency++;
            // Move to front only if using LRU policy
            if (cache->replacement_policy == lru_policy) {
                move_to_front(cache, entry->node);
            }
            return entry->node->value;
        }
        entry = entry->next;
    }
    
    return -1;  // Key not found
}

// Put a key-value pair in the cache
void put(Cache* cache, int key, int value) {
    if (!cache || !cache->replacement_policy) {
        return;
    }

    // Check if key exists
    unsigned int h = hash(key);
    HashEntry* entry = cache->hash_table[h];
    
    while (entry) {
        if (entry->key == key) {
            entry->node->value = value;
            entry->node->frequency++;
            // Move to front only if using LRU policy
            if (cache->replacement_policy == lru_policy) {
                move_to_front(cache, entry->node);
            }
            return;
        }
        entry = entry->next;
    }

    // Create new node
    LRUNode* new_node = create_node(key, value, cache);
    if (!new_node) {
        return;
    }

    // If cache is full, remove entry based on policy
    if (cache->size >= cache->capacity) {
        int remove_key = cache->replacement_policy(cache);
        if (remove_key != -1) {
            // Find and remove the node
            entry = cache->hash_table[hash(remove_key)];
            while (entry && entry->key != remove_key) {
                entry = entry->next;
            }
            if (entry) {
                remove_node(cache, entry->node);
                remove_from_hash(cache, remove_key);
                free(entry->node);
                cache->size--;
            }
        }
    }

    // Add new node
    add_to_front(cache, new_node);
    add_to_hash(cache, key, new_node);
    cache->size++;
}

// LRU replacement policy
int lru_policy(Cache* cache) {
    if (!cache || !cache->tail) {
        return -1;
    }
    return cache->tail->key;
}

// LFU replacement policy
int lfu_policy(Cache* cache) {
    if (!cache || !cache->head) {
        return -1;
    }

    LRUNode* current = cache->head;
    LRUNode* min_freq_node = current;
    int min_freq = current->frequency;

    while (current) {
        if (current->frequency < min_freq) {
            min_freq = current->frequency;
            min_freq_node = current;
        }
        current = current->next;
    }

    return min_freq_node->key;
}

// FIFO replacement policy
int fifo_policy(Cache* cache) {
    if (!cache || !cache->head) {
        return -1;
    }

    LRUNode* current = cache->head;
    LRUNode* oldest_node = current;
    int min_time = current->time_added;

    while (current) {
        if (current->time_added < min_time) {
            min_time = current->time_added;
            oldest_node = current;
        }
        current = current->next;
    }

    return oldest_node->key;
}

// Random replacement policy
int random_policy(Cache* cache) {
    if (!cache || !cache->head) {
        return -1;
    }

    // Count total nodes
    int count = 0;
    LRUNode* current = cache->head;
    while (current) {
        count++;
        current = current->next;
    }

    // Select random position
    int random_pos = rand() % count;
    
    // Find node at random position
    current = cache->head;
    while (random_pos > 0) {
        current = current->next;
        random_pos--;
    }

    return current->key;
}

void print_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Cache contents (Most Recent → Least Recent):\n");
    printf("------------------------------------------------\n");
    printf("Key\tValue\tFreq\tTime Added\n");
    printf("------------------------------------------------\n");
    
    LRUNode* current = cache->head;
    while (current) {
        printf("%d\t%d\t%d\t%d\n", 
               current->key, 
               current->value, 
               current->frequency, 
               current->time_added);
        current = current->next;
    }
    printf("------------------------------------------------\n");
    printf("Cache size: %d/%d\n", cache->size, cache->capacity);
}

void test_cache(Cache* cache, const char* policy_name) {
    printf("\nTesting %s policy:\n", policy_name);
    
    // Show initial empty cache state
    print_cache_contents(cache, "Initial cache state (empty)");
    
    // Add some test data
    put(cache, 1, 100);
    put(cache, 2, 200);
    put(cache, 3, 300);
    
    // Show cache state after initial insertions
    print_cache_contents(cache, "Cache state after initial insertions");
    
    // Test cache hits
    printf("\nTesting cache hits:\n");
    printf("Cache hit for key 1: %d\n", get(cache, 1));
    printf("Cache hit for key 2: %d\n", get(cache, 2));
    
    // Show cache state after hits
    print_cache_contents(cache, "Cache state after cache hits");
    
    // Test cache miss
    printf("\nTesting cache miss:\n");
    printf("Cache miss for key 4: %d\n", get(cache, 4));
    
    // Fill cache to capacity
    put(cache, 4, 400);
    put(cache, 5, 500);
    
    // Show cache state before replacement
    print_cache_contents(cache, "Cache state before replacement (at capacity)");
    
    // This will trigger replacement
    printf("\nTriggering replacement by adding key 6:\n");
    put(cache, 6, 600);
    
    // Show cache state after replacement
    print_cache_contents(cache, "Cache state after replacement");
}

void display_menu() {
    printf("\nCache Replacement Policy Menu:\n");
    printf("1. LRU (Least Recently Used)\n");
    printf("2. LFU (Least Frequently Used)\n");
    printf("3. FIFO (First In First Out)\n");
    printf("4. Random\n");
    printf("5. Run all policies\n");
    printf("6. Exit\n");
    printf("Enter your choice (1-6): ");
}

void run_interactive_mode() {
    int choice;
    int capacity;
    Cache* cache = NULL;
    
    printf("Enter cache capacity (1-%d): ", MAX_CACHE_SIZE);
    scanf("%d", &capacity);
    
    if (capacity <= 0 || capacity > MAX_CACHE_SIZE) {
        printf("Invalid capacity. Using default capacity of 5.\n");
        capacity = 5;
    }
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        
        if (choice == 6) {
            if (cache) {
                destroy_cache(cache);
            }
            break;
        }
        
        if (choice == 5) {
            // Run all policies
            Cache* lru_cache = create_cache(capacity);
            lru_cache->replacement_policy = lru_policy;
            test_cache(lru_cache, "LRU");
            destroy_cache(lru_cache);
            
            Cache* lfu_cache = create_cache(capacity);
            lfu_cache->replacement_policy = lfu_policy;
            test_cache(lfu_cache, "LFU");
            destroy_cache(lfu_cache);
            
            Cache* fifo_cache = create_cache(capacity);
            fifo_cache->replacement_policy = fifo_policy;
            test_cache(fifo_cache, "FIFO");
            destroy_cache(fifo_cache);
            
            Cache* random_cache = create_cache(capacity);
            random_cache->replacement_policy = random_policy;
            test_cache(random_cache, "Random");
            destroy_cache(random_cache);
            
            continue;
        }
        
        // Create new cache with selected policy
        if (cache) {
            destroy_cache(cache);
        }
        
        cache = create_cache(capacity);
        if (!cache) {
            printf("Failed to create cache. Exiting...\n");
            break;
        }
        
        switch (choice) {
            case 1:
                cache->replacement_policy = lru_policy;
                test_cache(cache, "LRU");
                break;
            case 2:
                cache->replacement_policy = lfu_policy;
                test_cache(cache, "LFU");
                break;
            case 3:
                cache->replacement_policy = fifo_policy;
                test_cache(cache, "FIFO");
                break;
            case 4:
                cache->replacement_policy = random_policy;
                test_cache(cache, "Random");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}

int main() {
    // Seed random number generator
    srand(time(NULL));
    
    printf("Welcome to Cache Replacement Policy Simulator\n");
    printf("===========================================\n");
    
    run_interactive_mode();
    
    return 0;
} 