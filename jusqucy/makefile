CC = gcc
CC_FLAGS = -shared -fPIC \
		   -I/usr/include/python3.11 \
		   -Wall -Wextra -Wconversion -Wno-unused-variable \
		   -Wno-unused-parameter -O2 -g

SOURCES = ../parser.c jusqucy.c 
OBJECTS = jusqucy.so

all: jusqucy.so

jusqucy.so: $(SOURCES)
	$(CC) $(CC_FLAGS) $(SOURCES) -o $@

clean:
	rm -f *.so *.o

.PHONY: all test clean 
