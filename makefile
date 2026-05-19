SRC := $(wildcard src/*.c) $(wildcard src/*.h)
CCFLAGS := -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter

all: jusquci.so

jusquci.so: $(SRC)
	gcc -fPIC -shared -o $@ $(CCFLAGS) $^

postgresql/jusquci.so cli/jusquci: $(SRC)
	$(MAKE) -C $(@D)

clean:
	rm -f jusquci.so
	$(MAKE) clean -C postgresql
	$(MAKE) clean -C cli
	rm -rf python/build python/*.egg-info

.PHONY: all clean
