# SRC := $(filter-out parser.c,$(wildcard *.c))
# OBJS := $(SRC:.c=.o)
OBJS = parser.o affixes.o punct.o util.o html.o
SRC := $(OBJS:.o=.c)
CCFLAGS := -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter

# all: $(OBJS)
#
# %.o: $(SRC)
# 	gcc -o $@ $(CCFLAGS) $< $(SRC)

parser.so: $(wildcard *.c)
	gcc -fPIC -shared -o $@ $(CCFLAGS) $(SRC)
