#include "cache_write.h"

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
    cache->write_policy = NULL;  // Must be set explicitly

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
        if (cache->entries[i].key == key && cache->entries[i].valid) {
            return i;
        }
    }
    return -1;
}

// Read value for a key from cache
int read(Cache* cache, int key) {
    int index = find_key(cache, key);
    if (index != -1) {
        return cache->entries[index].value;
    }
    return -1;  // Key not found
}

// Write a key-value pair in the cache
void write(Cache* cache, int key, int value) {
    if (!cache || !cache->write_policy) {
        return;
    }
    cache->write_policy(cache, key, value);
}

// Write-Through Policy
int write_through(Cache* cache, int key, int value) {
    int index = find_key(cache, key);
    
    // If key exists, update value
    if (index != -1) {
        cache->entries[index].value = value;
        cache->entries[index].last_modified = cache->current_time++;
        printf("Write-Through: Updated cache and memory for key %d\n", key);
        return 1;
    }

    // If cache is not full, add new entry
    if (cache->size < cache->capacity) {
        cache->entries[cache->size].key = key;
        cache->entries[cache->size].value = value;
        cache->entries[cache->size].valid = 1;
        cache->entries[cache->size].dirty = 0;
        cache->entries[cache->size].last_modified = cache->current_time++;
        cache->size++;
        printf("Write-Through: Added to cache and memory for key %d\n", key);
        return 1;
    }

    // Cache is full, need to evict an entry
    // For write-through, we can use any eviction policy
    // Here we'll use a simple FIFO-like approach
    int evict_index = 0;
    int oldest_time = cache->entries[0].last_modified;
    
    for (int i = 1; i < cache->size; i++) {
        if (cache->entries[i].last_modified < oldest_time) {
            oldest_time = cache->entries[i].last_modified;
            evict_index = i;
        }
    }

    cache->entries[evict_index].key = key;
    cache->entries[evict_index].value = value;
    cache->entries[evict_index].valid = 1;
    cache->entries[evict_index].dirty = 0;
    cache->entries[evict_index].last_modified = cache->current_time++;
    printf("Write-Through: Evicted old entry and wrote to cache and memory for key %d\n", key);
    return 1;
}

// Write-Back Policy
int write_back(Cache* cache, int key, int value) {
    int index = find_key(cache, key);
    
    // If key exists, update value and mark as dirty
    if (index != -1) {
        cache->entries[index].value = value;
        cache->entries[index].dirty = 1;
        cache->entries[index].last_modified = cache->current_time++;
        printf("Write-Back: Updated cache for key %d (marked dirty)\n", key);
        return 1;
    }

    // If cache is not full, add new entry
    if (cache->size < cache->capacity) {
        cache->entries[cache->size].key = key;
        cache->entries[cache->size].value = value;
        cache->entries[cache->size].valid = 1;
        cache->entries[cache->size].dirty = 1;
        cache->entries[cache->size].last_modified = cache->current_time++;
        cache->size++;
        printf("Write-Back: Added to cache for key %d (marked dirty)\n", key);
        return 1;
    }

    // Cache is full, need to evict an entry
    int evict_index = 0;
    int oldest_time = cache->entries[0].last_modified;
    
    for (int i = 1; i < cache->size; i++) {
        if (cache->entries[i].last_modified < oldest_time) {
            oldest_time = cache->entries[i].last_modified;
            evict_index = i;
        }
    }

    // If evicted entry is dirty, write it back to memory
    if (cache->entries[evict_index].dirty) {
        printf("Write-Back: Writing back dirty entry for key %d to memory\n", 
               cache->entries[evict_index].key);
    }

    cache->entries[evict_index].key = key;
    cache->entries[evict_index].value = value;
    cache->entries[evict_index].valid = 1;
    cache->entries[evict_index].dirty = 1;
    cache->entries[evict_index].last_modified = cache->current_time++;
    printf("Write-Back: Evicted old entry and wrote to cache for key %d (marked dirty)\n", key);
    return 1;
}

// Write-Around Policy
int write_around(Cache* cache, int key, int value) {
    int index = find_key(cache, key);
    
    // If key exists in cache, invalidate it
    if (index != -1) {
        cache->entries[index].valid = 0;
        printf("Write-Around: Invalidated cache entry for key %d\n", key);
    }

    // Write directly to memory
    printf("Write-Around: Writing directly to memory for key %d\n", key);
    return 1;
}

void print_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Key\tValue\tDirty\tValid\tLast Modified\n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < cache->size; i++) {
        printf("%d\t%d\t%d\t%d\t%d\n",
               cache->entries[i].key,
               cache->entries[i].value,
               cache->entries[i].dirty,
               cache->entries[i].valid,
               cache->entries[i].last_modified);
    }
    printf("--------------------------------------------------------\n");
}

void test_cache(Cache* cache, const char* policy_name) {
    printf("\nTesting %s policy:\n", policy_name);
    
    // Show initial empty cache state
    print_cache_contents(cache, "Initial cache state (empty)");
    
    // Add some test data
    write(cache, 1, 100);
    write(cache, 2, 200);
    write(cache, 3, 300);
    
    // Show cache state after initial writes
    print_cache_contents(cache, "Cache state after initial writes");
    
    // Test reads
    printf("\nTesting reads:\n");
    printf("Read key 1: %d\n", read(cache, 1));
    printf("Read key 2: %d\n", read(cache, 2));
    
    // Show cache state after reads
    print_cache_contents(cache, "Cache state after reads");
    
    // Test write to existing key
    printf("\nTesting write to existing key:\n");
    write(cache, 1, 150);
    
    // Show cache state after update
    print_cache_contents(cache, "Cache state after update");
    
    // Fill cache to capacity
    write(cache, 4, 400);
    write(cache, 5, 500);
    
    // Show cache state before eviction
    print_cache_contents(cache, "Cache state before eviction (at capacity)");
    
    // This will trigger eviction
    printf("\nTriggering eviction by writing key 6:\n");
    write(cache, 6, 600);
    
    // Show cache state after eviction
    print_cache_contents(cache, "Cache state after eviction");
}

void display_menu() {
    printf("\nCache Write Policy Menu:\n");
    printf("1. Write-Through\n");
    printf("2. Write-Back\n");
    printf("3. Write-Around\n");
    printf("4. Run all policies\n");
    printf("5. Exit\n");
    printf("Enter your choice (1-5): ");
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
        
        if (choice == 5) {
            if (cache) {
                destroy_cache(cache);
            }
            break;
        }
        
        if (choice == 4) {
            // Run all policies
            Cache* write_through_cache = create_cache(capacity);
            write_through_cache->write_policy = write_through;
            test_cache(write_through_cache, "Write-Through");
            destroy_cache(write_through_cache);
            
            Cache* write_back_cache = create_cache(capacity);
            write_back_cache->write_policy = write_back;
            test_cache(write_back_cache, "Write-Back");
            destroy_cache(write_back_cache);
            
            Cache* write_around_cache = create_cache(capacity);
            write_around_cache->write_policy = write_around;
            test_cache(write_around_cache, "Write-Around");
            destroy_cache(write_around_cache);
            
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
                cache->write_policy = write_through;
                test_cache(cache, "Write-Through");
                break;
            case 2:
                cache->write_policy = write_back;
                test_cache(cache, "Write-Back");
                break;
            case 3:
                cache->write_policy = write_around;
                test_cache(cache, "Write-Around");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}

int main() {
    printf("Welcome to Cache Write Policy Simulator\n");
    printf("=====================================\n");
    
    run_interactive_mode();
    
    return 0;
} 