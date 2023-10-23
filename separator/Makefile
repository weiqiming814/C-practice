all: thousend_separator test

CFLAGS= -Wall -g
server: thousend_separator.o
	gcc $(CFLAGS) $^ -o $@

test: test.o separator.o
	gcc $(CFLAGS) $^ -o $@

thousend_separator.o: thousend_separator.c

test.o: test.c

separator.o: separator.h

clean:
	@echo "cleanning project"
	-rm test separator thousend_separator *.o
	@echo "clean completed"

.PHONY: clean	
