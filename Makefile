CC ?= gcc
CFLAGS ?=
TEST_FLAGS := $(CFLAGS) -Wall -Werror -Wsign-compare -Wno-unknown-pragmas -Isrc

.PHONY: test

example:
	@$(CC) $(CFLAGS) -Isrc test/example.c src/*.c -o example -lm
	@./example
	@rm -f example

test-all:
	make test suite=duration
	make test suite=format
	make test suite=time

test:
	@$(CC) $(TEST_FLAGS) src/*.c test/$(suite).c -o $(suite).test -lm
	@./$(suite).test
	@rm -f $(suite).test
