CC = gcc
CC_FLAGS = -shared -fPIC \
		   -I/usr/include/python3.11 \
		   -Wall -Wextra -Wconversion -Wno-unused-variable \
		   -Wno-unused-parameter -O2 -g

SOURCES = 
OBJECTS = jusqucy.so

all: jusqucy.so presqucy.so

jusqucy.so: ../parser.c jusqucy.c 
	$(CC) $(CC_FLAGS) ../parser.c jusqucy.c -o $@

presqucy.so: presqucy.c
	$(CC) $(CC_FLAGS) presqucy.c -o $@

test: presqucy.so
	python3 -c "import presqucy; print(presqucy.normalize('auteur.rice.xs?'))"
	python3 -c "import presqucy; print(presqucy.normalize('a'))"

clean:
	rm -f *.so *.o

.PHONY: all test clean 
