all: thousend_separator test

CFLAGS= -Wall -g
server: thousend_separator.o
	gcc $(CFLAGS) $^ -Wall -g $@

server: test.o
	gcc $(CFLAGS) $^ -Wall -g $@

thousend_separator.o: thousend_separator.c

test.o: test.c

clean:
	@echo "cleanning project"
	-rm test thousend_separator *.o
	@echo "clean completed"

.PHONY: clean	
