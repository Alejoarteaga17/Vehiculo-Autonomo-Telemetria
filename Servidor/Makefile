CFLAGS=-O2 -Wall -Wextra -pthread
all: server
server: server.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm -f server