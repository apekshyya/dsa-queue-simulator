# dsa-queue-simulator
Traffic Light Simulation using Queues : A mini project that simulates traffic management at a junction using linear data structures with optional priority handling for high-traffic lanes.

## Descriptions :

This traffic simulation project developed in C language that implements queue data structures and uses SDL2 for visualization.This project consists of two main components:
1. A traffic generator that creates traffic patterns
2. A simulator that visualizes traffic flow using SDL2 graphics

The simulation demonstrates the practical application of queue data structures in traffic management, handling vehicle movement across multiple lanes.

## Components
1. Traffic Generator: Creates traffic patterns and writes them to lane files (laneA.txt, laneB.txt, laneC.txt, laneD.txt)
2. Traffic Simulator: Reads the generated traffic data, implements queue operations, and visualizes traffic using SDL2

## Prerequisites:
1. C compiler (Clang recommended)
2. SDL2 and SDL2_ttf libraries
3. Make

## Installations 

Step 1 : Clone the repository 
https://github.com/apekshyya/dsa-queue-simulator.git

Step 2 : Install Required Libraries
# On macOS (using Homebrew)
$ brew install sdl2 sdl2_ttf

# On Ubuntu/Debian
$ sudo apt-get install libsdl2-dev libsdl2-ttf-dev

Step 3 : Compile and Run
# Navigate to the project directory
$ cd traffic-simulator-queue

# Compile both components
$ make

# Run the generator
$ make run-generator

# Run the simulator
$ make run-simulator

# Run both applications (opens a new terminal for the generator)
$ make run

# Clean up compiled files and generated data
$ make clean

## Project Structure 

simulator.c - Main simulation program with SDL2 visualization
queue.c - Queue data structure implementation
traffic_generator.c - Program to generate traffic patterns
Makefile - Build configuration
