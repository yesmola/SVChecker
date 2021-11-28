.PHONY: build
build: configure
	cmake --build build

.PHONY: configure
configure:
	cmake -B build

.PHONY: run
run:
	./build/sol2inter

.PHONY: clean
clean:
	rm -rf build