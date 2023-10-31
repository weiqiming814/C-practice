all: client json_client

CFLAGS= -Wall -g

client: client.o
	gcc $(CFLAGS) $^ -o $@ -lcjson

json_client: json_client.o
	gcc $(CFLAGS) $^ -o $@ -lcjson

client.o: client.c

json_client.o: json_client.c

clean:
	@echo "cleanning project"
	-rm client json_client *.o
	@echo "clean completed"

.PHONY: clean	
