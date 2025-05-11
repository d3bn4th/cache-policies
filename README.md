# Cache Policies Implementation

This project implements various cache policies in C, including both replacement policies and write policies. The implementation is organized into two main components: cache replacement policies and cache write policies.

## Project Structure

```
cache_policies/
├── Makefile
├── replacement/
│   ├── cache_replacement.c
│   ├── cache_replacement.h
│   └── replacement_policy (executable)
└── write/
    ├── cache_write.c
    ├── cache_write.h
    └── write_policy (executable)
```

## Cache Replacement Policies

The project implements four different cache replacement policies:

1. **LRU (Least Recently Used)**
   - Evicts the entry that hasn't been accessed for the longest time
   - Uses a timestamp to track when each entry was last accessed

2. **LFU (Least Frequently Used)**
   - Evicts the entry that has been accessed the least number of times
   - Maintains a frequency counter for each entry

3. **FIFO (First In First Out)**
   - Evicts the oldest entry in the cache
   - Uses a simple queue-like structure

4. **Random**
   - Evicts a random entry from the cache
   - Provides a baseline for comparison

## Cache Write Policies

The project implements three different cache write policies:

1. **Write-Through**
   - Updates both cache and memory on every write
   - No dirty bits needed
   - Better for read-heavy workloads
   - Example output:
     ```
     Write-Through: Added to cache and memory for key 1
     Write-Through: Added to cache and memory for key 2
     ```

2. **Write-Back**
   - Updates only the cache initially
   - Marks entries as dirty when modified
   - Writes back to memory only when evicting dirty entries
   - Better for write-heavy workloads
   - Example output:
     ```
     Write-Back: Added to cache for key 1 (marked dirty)
     Write-Back: Writing back dirty entry for key 2 to memory
     ```

3. **Write-Around**
   - Writes directly to memory, bypassing cache
   - Only caches entries on read operations
   - Useful for one-time writes or streaming data
   - Example output:
     ```
     Write-Around: Written directly to memory for key 1
     Write-Around: Cache miss for key 1, reading from memory
     ```

## Building and Running

### Prerequisites
- GCC compiler
- Make

### Compilation
To compile both the replacement policy and write policy implementations:

```bash
cd cache_policies
make
```

This will create two executables:
- `replacement/replacement_policy`
- `replacement/write_policy`

### Running the Programs

1. **Cache Replacement Policy Simulator**:
```bash
./replacement/replacement_policy
```

2. **Cache Write Policy Simulator**:
```bash
./replacement/write_policy
```

## Example Usage

### Replacement Policy Example
```
Welcome to Cache Replacement Policy Simulator
===========================================
Enter cache capacity (1-100): 3

Cache Replacement Policy Menu:
1. LRU
2. LFU
3. FIFO
4. Random
5. Run all policies
6. Exit
Enter your choice (1-6): 1

Testing LRU policy:
...
```

### Write Policy Example
```
Welcome to Cache Write Policy Simulator
=====================================
Enter cache capacity (1-100): 3

Cache Write Policy Menu:
1. Write-Through
2. Write-Back
3. Write-Around
4. Run all policies
5. Exit
Enter your choice (1-5): 2

Testing Write-Back policy:
...
```

## Understanding the Output

### Cache State Display
The cache state is displayed in a table format:
```
Key     Value   Dirty   Valid   Last Modified
--------------------------------------------------------
1       100     0       1       0
2       200     0       1       1
3       300     0       1       2
--------------------------------------------------------
```

Where:
- `Key`: The identifier for the cache entry
- `Value`: The stored value
- `Dirty`: Whether the entry has been modified (1) or not (0)
- `Valid`: Whether the entry is valid (1) or not (0)
- `Last Modified`: Timestamp of the last modification

### Policy-Specific Output
Each policy provides specific output messages:
- Write-Through: Shows when entries are written to both cache and memory
- Write-Back: Shows when entries are marked dirty and written back to memory
- Write-Around: Shows when entries are written to memory and cached on read

## Interactive Features

Both simulators provide:
1. Interactive menu system
2. Before/after comparison of cache states
3. User-specified cache capacity
4. Option to run all policies or test individual ones
5. Detailed output showing cache state changes

## Cleaning Up

To remove compiled executables:
```bash
make clean
``` 