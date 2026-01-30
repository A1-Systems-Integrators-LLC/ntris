# Makefile for ntris tests

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -I..
LDFLAGS = -lncurses

# Test binaries
TESTS = test_sound

# Source files
SOUND_SRC = ../src/sound.c

all: $(TESTS)

test_sound: test_sound.c $(SOUND_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run all tests
test: $(TESTS)
	@echo "Running all tests..."
	@for test in $(TESTS); do \
		echo ""; \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "All tests passed!"

clean:
	rm -f $(TESTS)

.PHONY: all test clean
