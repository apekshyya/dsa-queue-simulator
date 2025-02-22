

# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -g -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -lm

# Target executables
SIMULATOR = simulator
GENERATOR = generator

# Source files
SIMULATOR_SRCS = simulator.c
GENERATOR_SRCS = traffic_generator.c
SIMULATOR_OBJS = $(SIMULATOR_SRCS:.c=.o)
GENERATOR_OBJS = $(GENERATOR_SRCS:.c=.o)

# Default target - build both programs
all: $(SIMULATOR) $(GENERATOR)

# Linking the simulator
$(SIMULATOR): $(SIMULATOR_OBJS)
	$(CC) $(SIMULATOR_OBJS) -o $(SIMULATOR) $(LDFLAGS)

# Linking the generator (note: no SDL flags needed)
$(GENERATOR): $(GENERATOR_OBJS)
	$(CC) $(GENERATOR_OBJS) -o $(GENERATOR)

# Compiling source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(SIMULATOR_OBJS) $(GENERATOR_OBJS) $(SIMULATOR) $(GENERATOR) laneA.txt laneB.txt laneC.txt laneD.txt

# Run targets
run-simulator: $(SIMULATOR)
	./$(SIMULATOR)

run-generator: $(GENERATOR)
	./$(GENERATOR)

# Run both programs (needs two terminal windows)
run:
	@echo "Starting Traffic Generator..."
	@osascript -e 'tell application "Terminal" to do script "cd $(shell pwd) && ./$(GENERATOR)"'
	@echo "Starting Simulator..."
	./$(SIMULATOR)

# Phony targets
.PHONY: all clean run run-simulator run-generator









