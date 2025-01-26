CC = gcc
SOURCES = ../src/*.c main.c
CCFLAGS = -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter

jusquci: $(SOURCES)
	$(CC) -Wall -o jusquci $(SOURCES) $(CCFLAGS)

test: jusquci
	# ./jusquci < ../tests/sentences.csv
	echo "humaine.e.s. ici? entends-les; À vaison-la-romaine" | ./jusquci 
	echo "entends-les" | ./jusquci 

valgrind: jusquci
	valgrind --leak-check=full ./jusquci < ../tests/sentences.csv 1>/dev/null
	valgrind --leak-check=full ./jusquci < ../LICENSE 1>/dev/null
	echo "entends-les" | valgrind --leak-check=full ./jusquci

clean:
	rm -f jusquci

.PHONY: test clean valgrind
