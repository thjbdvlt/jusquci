PREFIX  := /usr/local/bin

CC      := gcc
SOURCES := ../src/*.c main.c
CCFLAGS := -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter

.PHONY: install test amp valgrind clean

jusquci: $(SOURCES)
	$(CC) -Wall -o jusquci $(SOURCES) $(CCFLAGS)

install: jusquci
	cp $< $(PREFIX)/

test: jusquci
	# ./jusquci < ../tests/sentences.csv
	echo "humaine.e.s. ici? entends-les; À vaison-la-romaine" | ./jusquci 
	echo "entends-les" | ./jusquci 
	echo "aaaaa(quoi)rien aaaa( aaaa(e aaaa(je croi" | ./jusquci 
	echo "mais prêter[iez]-vous qqch à qqn? ou non[ quoi?" | ./jusquci 
	echo "mais (quoi) (qui?) (co)mment?" | ./jusquci 
	echo "(mais.) (mais). mais )." | ./jusquci 

amp: jusquci
	@echo "et&quoi? et&amp; &lg; -> &lg; a&a" | ./jusquci 
	@echo "et&quoi? et&amp; &lg; -> être a&a" | ./jusquci 
	@echo "et&quoi? et&amp; &lg; :: &lg; a&a" | ./jusquci 
	@echo "yo&lg;! &lg; a&a" | ./jusquci 
	@echo "et&amp;aaa &lg;! &lg; a&a" | ./jusquci 
	@echo "et&amp;aaa &lg;iii &lg; a&a" | ./jusquci 
	@echo "et&quoi? et&amp; &lg; aaa -> être a&a" | ./jusquci 
	@echo "et&quoi? et&amp; &lg; aaa . être a&a" | ./jusquci 
	@echo "et-> être aa" | ./jusquci 
	@echo "&a; a.a" | ./jusquci 
	@echo "et&quoi? et&#142; &#3; aaa . être a&a" | ./jusquci 
	@echo "et&quoi? et&#142;! &#3;:-) aaa . être a&a" | ./jusquci 

valgrind: jusquci
	valgrind --leak-check=full ./jusquci < ../tests/sentences.csv 1>/dev/null
	valgrind --leak-check=full ./jusquci < ../LICENSE 1>/dev/null
	echo "entends-les" | valgrind --leak-check=full ./jusquci

clean:
	rm -f jusquci
