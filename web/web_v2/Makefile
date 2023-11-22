all: server client

CFLAGS= -Wall -g
server: server.o
	gcc $(CFLAGS) $^ -lzlog -o $@

client: client.o
	gcc $(CFLAGS) $^ -o $@

server.o: server.c

client.o: client.c

clean:
	@echo "cleanning project"
	-rm server client *.o
	@echo "clean completed"

.PHONY: clean	
