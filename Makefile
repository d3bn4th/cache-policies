CC = gcc
CFLAGS = -Wall -Wextra

all: replacement/write_policy replacement/replacement_policy

replacement/write_policy: write/cache_write.c write/cache_write.h
	$(CC) $(CFLAGS) -o $@ write/cache_write.c

replacement/replacement_policy: replacement/cache_replacement.c replacement/cache_replacement.h
	$(CC) $(CFLAGS) -o $@ replacement/cache_replacement.c

clean:
	rm -f replacement/write_policy replacement/replacement_policy

.PHONY: all clean 