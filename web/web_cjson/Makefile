all: client

CFLAGS= -Wall -g

client: client.o
	gcc $(CFLAGS) $^ -o $@ -lcjson

client.o: client.c

clean:
	@echo "cleanning project"
	-rm client *.o
	@echo "clean completed"

.PHONY: clean	
