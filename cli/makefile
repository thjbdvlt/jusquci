CC = gcc
SOURCES = ../src/*.c main.c
CCFLAGS = -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter

jusquci: $(SOURCES)
	$(CC) -Wall -o jusquci $(SOURCES) $(CCFLAGS)

test: jusquci
	./jusquci < ../tests/sentences.csv

valgrind: jusquci
	valgrind --leak-check=full ./jusquci < ../tests/sentences.csv 1>/dev/null
	valgrind --leak-check=full ./jusquci < ../LICENSE 1>/dev/null

clean:
	rm -f jusquci

.PHONY: test clean valgrind
