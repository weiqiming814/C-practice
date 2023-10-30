all: url

CFLAGS= -Wall -g -DURL_PARSE_TEST
url: url.o
	gcc $(CFLAGS) $^ -o $@

url.o: url.c

clean:
	@echo "cleanning project"
	-rm url *.o
	@echo "clean completed"

.PHONY: clean	
