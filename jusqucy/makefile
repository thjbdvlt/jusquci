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
	python3 -c "import jusqucy; print(jusqucy.tokenize('alors? pourquoi pas ça? oui'))"
	python3 -c "import jusqucy; print(jusqucy.tokenize('éééte auteur·rice·x et· et les.euse.s'))"
	python3 -c "import jusqucy; print(jusqucy.tokenize('les humain.e.s sont là'))"
	python3 -c "import jusqucy; print(*jusqucy.tokenize('les autres\n\n\n...?\net qui? oui'))"


clean:
	rm -f *.so *.o

.PHONY: all test clean 
