CC = gcc
CC_FLAGS = -shared -fPIC \
		   -I/usr/include/python3.11 \
		   -Wall -Wextra -Wconversion -Wno-unused-variable \
		   -Wno-unused-parameter -O2 -g

SOURCES = ../src/*.c jusqucy.c
OBJECTS = jusqucy.so

all: jusqucy.so

jusqucy.so: ../src/parser.c jusqucy.c 
	$(CC) $(CC_FLAGS) $(SOURCES) -o $@

test: jusqucy.so
	python3 -c "import jusqucy; print(jusqucy.tokenize('a'))"
	python3 -c "import jusqucy; print(jusqucy.tokenize('éééte auteur·rice·x'))"

clean:
	rm -f *.so *.o

.PHONY: all test clean 
