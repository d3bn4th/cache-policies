#include "cache_replacement.h"

// Create a new cache with specified capacity
Cache* create_cache(int capacity) {
    if (capacity <= 0 || capacity > MAX_CACHE_SIZE) {
        return NULL;
    }

    Cache* cache = (Cache*)malloc(sizeof(Cache));
    if (!cache) {
        return NULL;
    }

    cache->entries = (CacheEntry*)calloc(capacity, sizeof(CacheEntry));
    if (!cache->entries) {
        free(cache);
        return NULL;
    }

    cache->size = 0;
    cache->capacity = capacity;
    cache->current_time = 0;
    cache->replacement_policy = NULL;  // Must be set explicitly

    return cache;
}

// Destroy the cache and free memory
void destroy_cache(Cache* cache) {
    if (cache) {
        free(cache->entries);
        free(cache);
    }
}

// Find a key in the cache
static int find_key(Cache* cache, int key) {
    for (int i = 0; i < cache->size; i++) {
        if (cache->entries[i].key == key) {
            return i;
        }
    }
    return -1;
}

// Get value for a key from cache
int get(Cache* cache, int key) {
    int index = find_key(cache, key);
    if (index != -1) {
        cache->entries[index].last_used = cache->current_time++;
        cache->entries[index].frequency++;
        return cache->entries[index].value;
    }
    return -1;  // Key not found
}

// Put a key-value pair in the cache
void put(Cache* cache, int key, int value) {
    if (!cache || !cache->replacement_policy) {
        return;
    }

    int index = find_key(cache, key);
    
    // If key exists, update value
    if (index != -1) {
        cache->entries[index].value = value;
        cache->entries[index].last_used = cache->current_time++;
        cache->entries[index].frequency++;
        return;
    }

    // If cache is not full, add new entry
    if (cache->size < cache->capacity) {
        cache->entries[cache->size].key = key;
        cache->entries[cache->size].value = value;
        cache->entries[cache->size].frequency = 1;
        cache->entries[cache->size].last_used = cache->current_time;
        cache->entries[cache->size].time_added = cache->current_time++;
        cache->size++;
        return;
    }

    // Cache is full, need to replace an entry
    int replace_index = cache->replacement_policy(cache);
    cache->entries[replace_index].key = key;
    cache->entries[replace_index].value = value;
    cache->entries[replace_index].frequency = 1;
    cache->entries[replace_index].last_used = cache->current_time;
    cache->entries[replace_index].time_added = cache->current_time++;
}

// LRU (Least Recently Used) replacement policy
int lru_policy(Cache* cache) {
    int lru_index = 0;
    int min_time = cache->entries[0].last_used;

    for (int i = 1; i < cache->size; i++) {
        if (cache->entries[i].last_used < min_time) {
            min_time = cache->entries[i].last_used;
            lru_index = i;
        }
    }
    return lru_index;
}

// LFU (Least Frequently Used) replacement policy
int lfu_policy(Cache* cache) {
    int lfu_index = 0;
    int min_freq = cache->entries[0].frequency;

    for (int i = 1; i < cache->size; i++) {
        if (cache->entries[i].frequency < min_freq) {
            min_freq = cache->entries[i].frequency;
            lfu_index = i;
        }
    }
    return lfu_index;
}

// FIFO (First In First Out) replacement policy
int fifo_policy(Cache* cache) {
    int fifo_index = 0;
    int min_time = cache->entries[0].time_added;

    for (int i = 1; i < cache->size; i++) {
        if (cache->entries[i].time_added < min_time) {
            min_time = cache->entries[i].time_added;
            fifo_index = i;
        }
    }
    return fifo_index;
}

// Random replacement policy
int random_policy(Cache* cache) {
    return rand() % cache->size;
}

void print_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Key\tValue\tFrequency\tLast Used\tTime Added\n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < cache->size; i++) {
        printf("%d\t%d\t%d\t\t%d\t\t%d\n",
               cache->entries[i].key,
               cache->entries[i].value,
               cache->entries[i].frequency,
               cache->entries[i].last_used,
               cache->entries[i].time_added);
    }
    printf("--------------------------------------------------------\n");
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