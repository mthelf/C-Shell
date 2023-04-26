all: build

build:
	@echo "Building"
	cc shell.c -o shell
	./shell

clean:
	@echo "Cleaning"
	rm shell
