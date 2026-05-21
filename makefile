SRC := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
CCFLAGS := -Wall -Wextra -Wconversion -Wno-unused-variable -Wno-unused-parameter
OBJS := $(SRC:.c=.o)

bindings := postgresql/jusquci.so \
						cli/jusquci \
						rust/target/release/libjusquci.rlib

all: jusquci.so

jusquci.so: $(SRC) $(HEADERS)
	gcc -fPIC -shared -o $@ $(CCFLAGS) $^

# %.o: %.c %.h
# 	gcc -o $@ $(CCFLAGS) $<

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
