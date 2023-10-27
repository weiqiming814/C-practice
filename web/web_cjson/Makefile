all: client

CFLAGS= -Wall -g

client: client.o cJSON.o
	gcc $(CFLAGS) $^ -o $@

client.o: client.c

cJSON.o: cJSON.c

clean:
	@echo "cleanning project"
	-rm client cJSON *.o
	@echo "clean completed"

.PHONY: clean	
