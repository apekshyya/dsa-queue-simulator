#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// Constants for lanes
#define NUM_ROADS 4
#define LANES_PER_ROAD 3
#define MAX_VEHICLES_PRIORITY 10
#define MIN_VEHICLES_PRIORITY 5

// Structure to represent a vehicle
typedef struct {
    char number[9];
    char road;
    int lane;
    int priority;
} Vehicle;

// Function to generate a random vehicle number
void generateVehicleNumber(char* buffer) {
    buffer[0] = 'A' + rand() % 26;
    buffer[1] = 'A' + rand() % 26;
    buffer[2] = '0' + rand() % 10;
    buffer[3] = 'A' + rand() % 26;
    buffer[4] = 'A' + rand() % 26;
    buffer[5] = '0' + rand() % 10;
    buffer[6] = '0' + rand() % 10;
    buffer[7] = '0' + rand() % 10;
    buffer[8] = '\0';
}

// Function to generate random road
char generateRoad() {
    char roads[] = {'A', 'B', 'C', 'D'};
    return roads[rand() % NUM_ROADS];
}

// Function to generate random lane number (1-3)
int generateLane() {
    return (rand() % LANES_PER_ROAD) + 1;
}

// Function to write vehicle data to road-specific file
void writeVehicleToFile(Vehicle* vehicle) {
    char filename[20];
    snprintf(filename, sizeof(filename), "lane%c.txt", vehicle->road);
    
    FILE* file = fopen(filename, "a");
    if (!file) {
        perror("Error opening file");
        return;
    }
    
    fprintf(file, "%s:%c%d:%d\n", 
            vehicle->number, 
            vehicle->road, 
            vehicle->lane,
            vehicle->priority);
    
    fflush(file);
    fclose(file);
}

// Function to count vehicles in a specific lane file
int countVehiclesInLane(char road, int lane) {
    char filename[20];
    snprintf(filename, sizeof(filename), "lane%c.txt", road);
    
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    int count = 0;
    char line[50];
    char targetLane[3];
    snprintf(targetLane, sizeof(targetLane), "%c%d", road, lane);
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, targetLane)) {
            count++;
        }
    }
    
    fclose(file);
    return count;
}

// Function to check if lane needs priority
int checkPriorityStatus(char road, int lane) {
    int vehicleCount = countVehiclesInLane(road, lane);
    return (vehicleCount >= MAX_VEHICLES_PRIORITY);
}

int main() {
    srand(time(NULL));
    
    // Initialize or clear existing lane files
    char roads[] = {'A', 'B', 'C', 'D'};
    for (int i = 0; i < NUM_ROADS; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "lane%c.txt", roads[i]);
        FILE* file = fopen(filename, "w");
        if (file) fclose(file);
    }
    
    while (1) {
        Vehicle vehicle;
        generateVehicleNumber(vehicle.number);
        vehicle.road = generateRoad();
        vehicle.lane = generateLane();
        
        // Check priority for lane AL2 (A road, lane 2)
        if (vehicle.road == 'A' && vehicle.lane == 2) {
            vehicle.priority = checkPriorityStatus('A', 2) ? 1 : 0;
        } else {
            vehicle.priority = 0;
        }
        
        // Write vehicle to appropriate file
        writeVehicleToFile(&vehicle);
        
        // Print generated vehicle info to console
        printf("Generated Vehicle - Number: %s, Road: %c, Lane: %d, Priority: %d\n",
               vehicle.number, vehicle.road, vehicle.lane, vehicle.priority);
        
        // Random delay between 1-3 seconds
        sleep(1 + (rand() % 3));
    }
    
    return 0;
}