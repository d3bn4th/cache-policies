#include <stdio.h>
#include <stdlib.h>
#include "replacement_algorithms/cache_interface.h"

#define CACHE_SIZE 3  // Fixed cache size to demonstrate replacement

void print_menu() {
    printf("\nCache Replacement Algorithm Tester\n");
    printf("=================================\n");
    printf("1. LRU (Least Recently Used)\n");
    printf("2. LFU (Least Frequently Used)\n");
    printf("3. FIFO (First In First Out)\n");
    printf("4. Random Replacement\n");
    printf("5. Run All Algorithms\n");
    printf("0. Exit\n");
    printf("Enter your choice: ");
}

void test_lru_cache(Cache* cache) {
    printf("\n=== Testing LRU Cache ===\n");
    put_lru(cache, 1, 100);
    put_lru(cache, 2, 200);
    put_lru(cache, 3, 300);
    print_lru_cache_contents(cache, "After initial insertions (1,2,3)");
    
    printf("Getting key 1: %d\n", get_lru(cache, 1));
    printf("Getting key 2: %d\n", get_lru(cache, 2));
    printf("Getting key 1 again: %d\n", get_lru(cache, 1));
    print_lru_cache_contents(cache, "After accessing 1,2,1");
    
    put_lru(cache, 4, 400);
    print_lru_cache_contents(cache, "After adding 4 (might trigger replacement)");
    
    int result = get_lru(cache, 999);
    printf("Getting non-existent key 999: %d\n", result);
    printf("=== End of LRU Cache Test ===\n\n");
}

void test_lfu_cache(Cache* cache) {
    printf("\n=== Testing LFU Cache ===\n");
    put_lfu(cache, 1, 100);
    put_lfu(cache, 2, 200);
    put_lfu(cache, 3, 300);
    print_lfu_cache_contents(cache, "After initial insertions (1,2,3)");
    
    printf("Getting key 1: %d\n", get_lfu(cache, 1));
    printf("Getting key 2: %d\n", get_lfu(cache, 2));
    printf("Getting key 1 again: %d\n", get_lfu(cache, 1));
    print_lfu_cache_contents(cache, "After accessing 1,2,1");
    
    put_lfu(cache, 4, 400);
    print_lfu_cache_contents(cache, "After adding 4 (might trigger replacement)");
    
    int result = get_lfu(cache, 999);
    printf("Getting non-existent key 999: %d\n", result);
    printf("=== End of LFU Cache Test ===\n\n");
}

void test_fifo_cache(Cache* cache) {
    printf("\n=== Testing FIFO Cache ===\n");
    put_fifo(cache, 1, 100);
    put_fifo(cache, 2, 200);
    put_fifo(cache, 3, 300);
    print_fifo_cache_contents(cache, "After initial insertions (1,2,3)");
    
    printf("Getting key 1: %d\n", get_fifo(cache, 1));
    printf("Getting key 2: %d\n", get_fifo(cache, 2));
    printf("Getting key 1 again: %d\n", get_fifo(cache, 1));
    print_fifo_cache_contents(cache, "After accessing 1,2,1");
    
    put_fifo(cache, 4, 400);
    print_fifo_cache_contents(cache, "After adding 4 (might trigger replacement)");
    
    int result = get_fifo(cache, 999);
    printf("Getting non-existent key 999: %d\n", result);
    printf("=== End of FIFO Cache Test ===\n\n");
}

void test_random_cache(Cache* cache) {
    printf("\n=== Testing Random Cache ===\n");
    put_random(cache, 1, 100);
    put_random(cache, 2, 200);
    put_random(cache, 3, 300);
    print_random_cache_contents(cache, "After initial insertions (1,2,3)");
    
    printf("Getting key 1: %d\n", get_random(cache, 1));
    printf("Getting key 2: %d\n", get_random(cache, 2));
    printf("Getting key 1 again: %d\n", get_random(cache, 1));
    print_random_cache_contents(cache, "After accessing 1,2,1");
    
    put_random(cache, 4, 400);
    print_random_cache_contents(cache, "After adding 4 (might trigger replacement)");
    
    int result = get_random(cache, 999);
    printf("Getting non-existent key 999: %d\n", result);
    printf("=== End of Random Cache Test ===\n\n");
}

void run_selected_algorithm(int choice) {
    Cache* cache = NULL;
    
    switch(choice) {
        case 1:
            cache = create_lru_cache(CACHE_SIZE);
            test_lru_cache(cache);
            destroy_lru_cache(cache);
            break;
            
        case 2:
            cache = create_lfu_cache(CACHE_SIZE);
            test_lfu_cache(cache);
            destroy_lfu_cache(cache);
            break;
            
        case 3:
            cache = create_fifo_cache(CACHE_SIZE);
            test_fifo_cache(cache);
            destroy_fifo_cache(cache);
            break;
            
        case 4:
            cache = create_random_cache(CACHE_SIZE);
            test_random_cache(cache);
            destroy_random_cache(cache);
            break;
            
        case 5:
            printf("\nRunning all cache replacement algorithms...\n");
            
            cache = create_lru_cache(CACHE_SIZE);
            test_lru_cache(cache);
            destroy_lru_cache(cache);
            
            cache = create_lfu_cache(CACHE_SIZE);
            test_lfu_cache(cache);
            destroy_lfu_cache(cache);
            
            cache = create_fifo_cache(CACHE_SIZE);
            test_fifo_cache(cache);
            destroy_fifo_cache(cache);
            
            cache = create_random_cache(CACHE_SIZE);
            test_random_cache(cache);
            destroy_random_cache(cache);
            break;
            
        default:
            printf("Invalid choice!\n");
    }
}

int main() {
    int choice;
    
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input! Please enter a number.\n");
            continue;
        }
        
        if (choice == 0) {
            printf("Exiting...\n");
            break;
        }
        
        if (choice >= 1 && choice <= 5) {
            run_selected_algorithm(choice);
        } else {
            printf("Invalid choice! Please select a number between 0 and 5.\n");
        }
        
        printf("\nPress Enter to continue...");
        while (getchar() != '\n'); // Clear any remaining newline
        getchar(); // Wait for Enter
        
    } while (1);
    
    return 0;
} 