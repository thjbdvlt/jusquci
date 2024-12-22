CC = gcc
CC_FLAGS = -shared -fPIC \
		   -I/usr/include/python3.11 \
		   -Wall -Wextra -Wconversion -Wno-unused-variable \
		   -Wno-unused-parameter -O2 -g

SOURCES = 
OBJECTS = jusqucy.so

all: jusqucy.so

jusqucy.so: ../parser.c jusqucy.c 
	$(CC) $(CC_FLAGS) ../parser.c jusqucy.c -o $@

test: jusqucy.so
	python3 -c "import jusqucy; print(jusqucy.tokenize('a'))"

clean:
	rm -f *.so *.o

.PHONY: all test clean 
