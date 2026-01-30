# ntris Makefile
# Build system for NES-style Tetris clone

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2
LDFLAGS = -lncurses

# Source directory
SRCDIR = src

# Source files (wildcard to pick up all .c files in src/)
SRCS = $(wildcard $(SRCDIR)/*.c)

# Object files (in src/ directory)
OBJS = $(SRCS:.c=.o)

# Target binary
TARGET = ntris

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Pattern rule for object files
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)

# PHONY targets
.PHONY: all clean
