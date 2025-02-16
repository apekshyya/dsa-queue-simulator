# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -g -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -lm

# Target executable name
TARGET = simulator

# Source files
SRCS = simulator.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compiling source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)

# Run target
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
