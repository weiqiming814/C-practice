all: server client

server: server.o
	gcc $^ -o $@

client: client.o
	gcc $^ -o $@

server.o: server.c

client.o: client.c

clean:
	@echo "cleanning project"
	-rm web *.o
	@echo "clean completed"

.PHONY: clean	
