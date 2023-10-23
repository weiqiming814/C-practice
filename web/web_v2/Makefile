all: server client hello

CFLAGS= -Wall -g
server: server.o
	gcc $(CFLAGS) $^ -o $@

client: client.o
	gcc $(CFLAGS) $^ -o $@

hello: hello.o
	gcc $(CFLAGS) $^ -o $@

hello.o: hello.c

server.o: server.c

client.o: client.c

clean:
	@echo "cleanning project"
	-rm server client hello *.o
	@echo "clean completed"

.PHONY: clean	
