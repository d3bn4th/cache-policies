CC = gcc
CFLAGS = -Wall -Wextra -I.
CACHE_SRCS = replacement_algorithms/lru_cache.c \
             replacement_algorithms/lfu_cache.c \
             replacement_algorithms/fifo_cache.c \
             replacement_algorithms/random_cache.c
CACHE_OBJS = $(CACHE_SRCS:.c=.o)

all: test_cache_algorithms

test_cache_algorithms: test_cache_algorithms.c $(CACHE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f test_cache_algorithms $(CACHE_OBJS)

.PHONY: all clean 