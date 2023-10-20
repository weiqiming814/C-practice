all: thousend_separator

CFLAGS= -Wall -g
server: thousend_separator.o
	gcc $(CFLAGS) $^ -o $@

thousend_separator.o: thousend_separator.c

clean:
	@echo "cleanning project"
	-rm thousend_separator *.o
	@echo "clean completed"

.PHONY: clean	
