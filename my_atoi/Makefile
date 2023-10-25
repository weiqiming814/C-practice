all: test

CFLAGS= -Wall -g

test: test.o my_atoi.o
	gcc $(CFLAGS) $^ -o $@

test.o: test.c

my_atoi.o: my_atoi.h

clean:
	@echo "cleanning project"
	-rm test my_atoi *.o
	@echo "clean completed"

.PHONY: clean	
