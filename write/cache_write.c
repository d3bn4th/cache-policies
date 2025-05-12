#include "cache_write.h"

// Global memory for simulation
Memory main_memory;

// Initialize memory
void init_memory() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        main_memory.data[i] = 0;
        main_memory.initialized[i] = 0;
    }
}

// Memory read operation
int memory_read(int address) {
    if (address < 0 || address >= MEMORY_SIZE) {
        printf("Memory error: Address %d out of bounds\n", address);
        return -1;
    }
    
    if (!main_memory.initialized[address]) {
        printf("Memory notice: Reading uninitialized address %d\n", address);
    }
    
    return main_memory.data[address];
}

// Memory write operation
void memory_write(int address, int value) {
    if (address < 0 || address >= MEMORY_SIZE) {
        printf("Memory error: Address %d out of bounds\n", address);
        return;
    }
    
    main_memory.data[address] = value;
    main_memory.initialized[address] = 1;
    printf("Memory write: Address %d = %d\n", address, value);
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
        // Write back any dirty entries before destroying
        if (cache->write_policy == write_back || cache->write_policy == write_back_no_allocate) {
            for (int i = 0; i < cache->size; i++) {
                if (cache->entries[i].valid && cache->entries[i].dirty) {
                    memory_write(cache->entries[i].key, cache->entries[i].value);
                    printf("Cache destruction: Writing back dirty entry for key %d\n", cache->entries[i].key);
                }
            }
        }
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
        printf("Cache hit: Reading key %d from cache\n", key);
        return cache->entries[index].value;
    }
    
    // Cache miss - read from memory
    printf("Cache miss: Reading key %d from memory\n", key);
    int value = memory_read(key);
    
    // For a read miss, we might want to load the value into cache
    // This is a simplified implementation without read allocation policy
    return value;
}

// Write a key-value pair in the cache
int write(Cache* cache, int key, int value) {
    if (!cache || !cache->write_policy) {
        return 0;
    }
    return cache->write_policy(cache, key, value);
}

// Write-Through Policy
int write_through(Cache* cache, int key, int value) {
    int index = find_key(cache, key);
    
    // Always write to memory first
    memory_write(key, value);
    
    // If key exists, update value
    if (index != -1) {
        cache->entries[index].value = value;
        cache->entries[index].last_modified = cache->current_time++;
        printf("Write-Through: Updated cache for key %d\n", key);
        return 1;
    }

    // If cache is not full, add new entry
    if (cache->size < cache->capacity) {
        cache->entries[cache->size].key = key;
        cache->entries[cache->size].value = value;
        cache->entries[cache->size].valid = 1;
        cache->entries[cache->size].dirty = 0;  // Not dirty since memory is updated
        cache->entries[cache->size].last_modified = cache->current_time++;
        cache->size++;
        printf("Write-Through: Added to cache for key %d\n", key);
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

    printf("Write-Through: Evicted old entry for key %d\n", cache->entries[evict_index].key);
    cache->entries[evict_index].key = key;
    cache->entries[evict_index].value = value;
    cache->entries[evict_index].valid = 1;
    cache->entries[evict_index].dirty = 0;  // Not dirty since memory is updated
    cache->entries[evict_index].last_modified = cache->current_time++;
    return 1;
}

// Write-Back Policy
int write_back(Cache* cache, int key, int value) {
    int index = find_key(cache, key);
    
    // If key exists, update value and mark as dirty
    if (index != -1) {
        cache->entries[index].value = value;
        cache->entries[index].dirty = 1;  // Mark as dirty, needs to be written to memory later
        cache->entries[index].last_modified = cache->current_time++;
        printf("Write-Back: Updated cache for key %d (marked dirty)\n", key);
        return 1;
    }

    // If cache is not full, add new entry
    if (cache->size < cache->capacity) {
        cache->entries[cache->size].key = key;
        cache->entries[cache->size].value = value;
        cache->entries[cache->size].valid = 1;
        cache->entries[cache->size].dirty = 1;  // Mark as dirty, needs to be written to memory later
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
        memory_write(cache->entries[evict_index].key, cache->entries[evict_index].value);
        printf("Write-Back: Writing back dirty entry for key %d to memory\n", 
               cache->entries[evict_index].key);
    } else {
        printf("Write-Back: Evicted clean entry for key %d (no memory write needed)\n",
               cache->entries[evict_index].key);
    }

    cache->entries[evict_index].key = key;
    cache->entries[evict_index].value = value;
    cache->entries[evict_index].valid = 1;
    cache->entries[evict_index].dirty = 1;  // Mark as dirty for new entry
    cache->entries[evict_index].last_modified = cache->current_time++;
    return 1;
}

// Write-Around Policy
int write_around(Cache* cache, int key, int value) {
    // Write directly to memory, bypassing cache
    memory_write(key, value);
    
    int index = find_key(cache, key);
    
    // If key exists in cache, invalidate it since memory now has newer value
    if (index != -1) {
        cache->entries[index].valid = 0;
        printf("Write-Around: Invalidated cache entry for key %d\n", key);
    }

    printf("Write-Around: Bypassed cache, wrote directly to memory for key %d\n", key);
    return 1;
}

// Write-Back with No-Write-Allocate
int write_back_no_allocate(Cache* cache, int key, int value) {
    // First, check if the key exists in cache
    int index = find_key(cache, key);
    
    if (index != -1) {
        // Key exists in cache, update it and mark as dirty
        cache->entries[index].value = value;
        cache->entries[index].dirty = 1;
        cache->entries[index].last_modified = cache->current_time++;
        printf("Write-Back No-Allocate: Updated cache for key %d (marked dirty)\n", key);
        return 1;
    }
    
    // Key doesn't exist in cache, write directly to memory
    // In no-write-allocate, we don't add the entry to cache on write miss
    memory_write(key, value);
    printf("Write-Back No-Allocate: Cache miss, written directly to memory for key %d\n", key);
    return 1;
}

// Write-Allocate Policy (with Write-Back)
int write_allocate(Cache* cache, int key, int value) {
    int index = find_key(cache, key);
    
    // If key exists, update value and mark as dirty (like write-back)
    if (index != -1) {
        cache->entries[index].value = value;
        cache->entries[index].dirty = 1;
        cache->entries[index].last_modified = cache->current_time++;
        printf("Write-Allocate: Updated cache for key %d (marked dirty)\n", key);
        return 1;
    }
    
    // Key doesn't exist - this is where write-allocate differs from no-write-allocate
    // First, read the value from memory (simulating loading the block)
    memory_read(key);  // We read from memory to load the block, but we'll use the new value anyway
    printf("Write-Allocate: Cache miss for key %d, loading block from memory\n", key);
    
    // Then add the block to cache (allocate) and update with new value
    if (cache->size < cache->capacity) {
        // Cache has space
        cache->entries[cache->size].key = key;
        cache->entries[cache->size].value = value;  // Use the new value
        cache->entries[cache->size].valid = 1;
        cache->entries[cache->size].dirty = 1;  // Mark dirty since we're modifying it
        cache->entries[cache->size].last_modified = cache->current_time++;
        cache->size++;
        printf("Write-Allocate: Allocated new cache entry for key %d and updated value (marked dirty)\n", key);
        return 1;
    }
    
    // No space in cache, need to evict
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
        memory_write(cache->entries[evict_index].key, cache->entries[evict_index].value);
        printf("Write-Allocate: Writing back dirty entry for key %d to memory\n", 
               cache->entries[evict_index].key);
    } else {
        printf("Write-Allocate: Evicted clean entry for key %d (no memory write needed)\n",
               cache->entries[evict_index].key);
    }
    
    // Replace with new entry
    cache->entries[evict_index].key = key;
    cache->entries[evict_index].value = value;
    cache->entries[evict_index].valid = 1;
    cache->entries[evict_index].dirty = 1;  // Mark dirty since we're modifying it
    cache->entries[evict_index].last_modified = cache->current_time++;
    printf("Write-Allocate: Allocated cache entry for key %d after eviction (marked dirty)\n", key);
    
    return 1;
}

void print_cache_contents(Cache* cache, const char* message) {
    printf("\n%s:\n", message);
    printf("Key\tValue\tDirty\tValid\tLast Modified\n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < cache->size; i++) {
        printf("%d\t%d\t%d\t%d\t%ld\n",
               cache->entries[i].key,
               cache->entries[i].value,
               cache->entries[i].dirty,
               cache->entries[i].valid,
               (long)cache->entries[i].last_modified);
    }
    printf("--------------------------------------------------------\n");
}

void print_memory_contents(int start_addr, int end_addr, const char* message) {
    printf("\n%s:\n", message);
    printf("Address\tValue\tInitialized\n");
    printf("--------------------------------------------------------\n");
    
    for (int addr = start_addr; addr <= end_addr; addr++) {
        if (main_memory.initialized[addr]) {
            printf("%d\t%d\t%s\n", addr, main_memory.data[addr], "Yes");
        }
    }
    printf("--------------------------------------------------------\n");
}

void test_cache(Cache* cache, const char* policy_name) {
    printf("\nTesting %s policy:\n", policy_name);
    
    // Initialize memory for this test
    init_memory();
    
    // Show initial empty cache state
    print_cache_contents(cache, "Initial cache state (empty)");
    
    // Add some test data
    write(cache, 1, 100);
    write(cache, 2, 200);
    write(cache, 3, 300);
    
    // Show cache and memory state after initial writes
    print_cache_contents(cache, "Cache state after initial writes");
    print_memory_contents(1, 10, "Memory state after initial writes");
    
    // Test reads
    printf("\nTesting reads:\n");
    printf("Read key 1: %d\n", read(cache, 1));
    printf("Read key 2: %d\n", read(cache, 2));
    
    // Test read miss
    printf("Read key 10 (should be miss): %d\n", read(cache, 10));
    
    // Show cache state after reads
    print_cache_contents(cache, "Cache state after reads");
    
    // Test write to existing key
    printf("\nTesting write to existing key:\n");
    write(cache, 1, 150);
    
    // Show cache and memory state after update
    print_cache_contents(cache, "Cache state after update");
    print_memory_contents(1, 10, "Memory state after update");
    
    // Fill cache to capacity
    write(cache, 4, 400);
    write(cache, 5, 500);
    
    // Show cache state before eviction
    print_cache_contents(cache, "Cache state before eviction (at capacity)");
    
    // This will trigger eviction
    printf("\nTriggering eviction by writing key 6:\n");
    write(cache, 6, 600);
    
    // Show cache and memory state after eviction
    print_cache_contents(cache, "Cache state after eviction");
    print_memory_contents(1, 10, "Memory state after eviction");
}

void display_menu() {
    printf("\nCache Write Policy Menu:\n");
    printf("1. Write-Through\n");
    printf("2. Write-Back\n");
    printf("3. Write-Around\n");
    printf("4. Write-Back with No-Write-Allocate\n");
    printf("5. Write-Allocate (with Write-Back)\n");
    printf("6. Run all policies\n");
    printf("7. Exit\n");
    printf("Enter your choice (1-7): ");
}

void run_interactive_mode(Cache* cache) {
    int choice;
    int capacity;
    
    printf("Enter cache capacity (1-%d): ", MAX_CACHE_SIZE);
    scanf("%d", &capacity);
    
    if (capacity <= 0 || capacity > MAX_CACHE_SIZE) {
        printf("Invalid capacity. Using default capacity of 5.\n");
        capacity = 5;
    }
    
    // Initialize memory at the start
    init_memory();
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        
        if (choice == 7) {  // Updated exit choice
            if (cache) {
                destroy_cache(cache);
            }
            break;
        }
        
        if (choice == 6) {  // Updated "Run all" choice
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
            
            Cache* write_back_no_allocate_cache = create_cache(capacity);
            write_back_no_allocate_cache->write_policy = write_back_no_allocate;
            test_cache(write_back_no_allocate_cache, "Write-Back with No-Write-Allocate");
            destroy_cache(write_back_no_allocate_cache);
            
            Cache* write_allocate_cache = create_cache(capacity);
            write_allocate_cache->write_policy = write_allocate;
            test_cache(write_allocate_cache, "Write-Allocate (with Write-Back)");
            destroy_cache(write_allocate_cache);
            
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
        
        // Reset memory for each new test
        init_memory();
        
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
            case 4:
                cache->write_policy = write_back_no_allocate;
                test_cache(cache, "Write-Back with No-Write-Allocate");
                break;
            case 5:  // New option for Write-Allocate
                cache->write_policy = write_allocate;
                test_cache(cache, "Write-Allocate (with Write-Back)");
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
    
    // Initialize memory at program start
    init_memory();
    
    Cache* cache = NULL;
    run_interactive_mode(cache);
    
    return 0;
} 