so := ../jusquci.so

build: $(so)
	cargo build --release

clean:
	cargo clean

test: $(so)
	cargo test

$(so):
	$(MAKE) -C ..

.PHONY: build clean test $(so)
