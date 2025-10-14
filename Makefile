CC ?= gcc
CFLAGS ?=
SRC_FLAGS := -Isrc $(CFLAGS) -std=c11 -pedantic -Wall -Werror -Wextra -Wshadow -Wsign-compare -Wstrict-prototypes -Wunused
TEST_FLAGS := -Wno-missing-field-initializers

.PHONY: test

run-example:
	@$(CC) $(CFLAGS) -Isrc test/example.c src/*.c -o example -lm
	@./example
	@rm -f example

test-all:
	make test suite=duration
	make test suite=format
	make test suite=time

test:
	$(CC) $(SRC_FLAGS) src/*.c $(TEST_FLAGS) test/$(suite).c -o $(suite).test -lm
	@./$(suite).test
	@rm -f $(suite).test
