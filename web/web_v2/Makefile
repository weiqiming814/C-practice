all: server client

CFLAGS= -Wall -g
server: server.o common.o
	gcc $(CFLAGS) $^ -lzlog -o $@

client: client.o
	gcc $(CFLAGS) $^ -o $@

server.o: server.c

client.o: client.c

common.o: common.h

clean:
	@echo "cleanning project"
	-rm server client common *.o
	@echo "clean completed"

.PHONY: clean	
