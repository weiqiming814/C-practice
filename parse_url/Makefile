all: url

CFLAGS= -Wall -g
url: url.o
	gcc $(CFLAGS) $^ -o $@

url.o: url.c

clean:
	@echo "cleanning project"
	-rm url *.o
	@echo "clean completed"

.PHONY: clean	
