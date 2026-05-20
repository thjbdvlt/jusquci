SRC := $(wildcard src/*.c) $(wildcard src/*.h)
CCFLAGS := -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter

bindings := postgresql/jusquci.so \
						cli/jusquci \
						rust/target/release/libjusquci.rlib

all: jusquci.so

jusquci.so: $(SRC)
	gcc -fPIC -shared -o $@ $(CCFLAGS) $^

bindings: $(bindings)

$(bindings): $(SRC)
	$(MAKE) -C $(@D)

clean:
	rm -f jusquci.so
	$(MAKE) clean -C postgresql
	$(MAKE) clean -C cli
	$(MAKE) clean -C rust
	rm -rf python/build python/*.egg-info

.PHONY: all clean bindings
