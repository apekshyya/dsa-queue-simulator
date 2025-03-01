#define SDL_MAIN_HANDLED  // Prevent SDL from overriding main
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include "queue.h"  // Include the queue header

const int SCREEN_WIDTH = 1000;  // Increased screen width (wider screen)
const int SCREEN_HEIGHT = 800;  // Keeping the same height

// Lane width for each lane
const int LANE_WIDTH = SCREEN_WIDTH / 9;

// Lane division colors (light and subtle)
SDL_Color laneDivisionColor = {180, 180, 180, 255};  // Light gray for subtle divisions

// Traffic light states
typedef enum {
    RED,
    GREEN
} LightState;

// Traffic light structure
typedef struct {
    int x;
    int y;
    int radius;
    LightState state;
    Uint32 lastToggleTime;  // Time when last toggled
    Uint32 duration;        // Duration before next toggle (in milliseconds)
    bool isPriority;        // Flag to indicate if this lane has priority
} TrafficLight;

// Vehicle structure to hold properties of a vehicle
typedef struct {
    SDL_Rect rect;  // Rectangle for the vehicle (position and size)
    int speed;      // Speed of the vehicle
    int targetX;    // Target X position (destination)
    int targetY;    // Target Y position (destination)
    bool active;    // Whether vehicle is active/visible
    char road;      // Road identifier (A, B, C, D)
    int lane;       // Lane number (1, 2, 3)
    bool isPriority; // Whether this vehicle is in a priority lane
    char number[9]; // Vehicle number for identification
} Vehicle;

// Queue structure for vehicle generation
typedef struct {
    Vehicle* vehicles;
    int capacity;
    int size;
    int front;
    int rear;
    Uint32 lastGenerationTime;
    Uint32 generationInterval;
    char road;      // Road identifier for this queue
    int lane;       // Lane number for this queue
} VehicleQueue;

// Declare the drawCircle function
void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius) {
    // Draw a circle using the midpoint circle algorithm
    for (int w = 0; w < 360; w++) {
        int x = (int)(centerX + radius * cos(w * M_PI / 180.0));
        int y = (int)(centerY + radius * sin(w * M_PI / 180.0));
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

// Initialize traffic light
TrafficLight initTrafficLight(int x, int y, int radius, LightState initialState, Uint32 duration) {
    TrafficLight light;
    light.x = x;
    light.y = y;
    light.radius = radius;
    light.state = initialState;
    light.lastToggleTime = SDL_GetTicks();
    light.duration = duration;
    light.isPriority = false;
    return light;
}

// Toggle traffic light state
void toggleTrafficLight(TrafficLight* light) {
    if (light->state == RED) {
        light->state = GREEN;
    } else {
        light->state = RED;
    }
    light->lastToggleTime = SDL_GetTicks();
}

// Update traffic light state based on time
void updateTrafficLight(TrafficLight* light) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - light->lastToggleTime >= light->duration) {
        toggleTrafficLight(light);
    }
}

// Initialize vehicle queue
VehicleQueue initVehicleQueue(int capacity, Uint32 generationInterval, char road, int lane) {
    VehicleQueue queue;
    queue.capacity = capacity;
    queue.vehicles = (Vehicle*)malloc(capacity * sizeof(Vehicle));
    queue.size = 0;
    queue.front = 0;
    queue.rear = -1;
    queue.lastGenerationTime = SDL_GetTicks();
    queue.generationInterval = generationInterval;
    queue.road = road;
    queue.lane = lane;
    
    // Initialize all vehicles as inactive
    for (int i = 0; i < capacity; i++) {
        queue.vehicles[i].active = false;
    }
    
    return queue;
}

// Add vehicle to queue
bool enqueueVehicle(VehicleQueue* queue, Vehicle vehicle) {
    if (queue->size == queue->capacity) {
        return false;  // Queue is full
    }
    
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->vehicles[queue->rear] = vehicle;
    queue->size++;
    return true;
}

// Remove vehicle from queue
bool dequeueVehicle(VehicleQueue* queue, Vehicle* vehicle) {
    if (queue->size == 0) {
        return false;  // Queue is empty
    }
    
    *vehicle = queue->vehicles[queue->front];
    queue->vehicles[queue->front].active = false;
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return true;
}

// Generate a random vehicle number
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

// Function to generate vehicles for middle lanes
void generateMiddleLaneVehicles(VehicleQueue* queues, int queueCount) {
    Uint32 currentTime = SDL_GetTicks();
    
    for (int i = 0; i < queueCount; i++) {
        if (currentTime - queues[i].lastGenerationTime >= queues[i].generationInterval) {
            // Generate vehicle for middle lane
            Vehicle newVehicle;
            newVehicle.active = true;
            newVehicle.speed = 4;
            newVehicle.road = queues[i].road;
            newVehicle.lane = queues[i].lane;
            newVehicle.isPriority = (newVehicle.road == 'A' && newVehicle.lane == 2); // A2 is priority lane
            generateVehicleNumber(newVehicle.number);
            
            // Initialize position based on which middle lane
            switch (queues[i].road) {
                case 'A':  // A2 Middle Lane (Top vertical)
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5 - 20, 0 - 40, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5 - 20;
                    newVehicle.targetY = SCREEN_HEIGHT / 3 - 50;
                    break;
                case 'B':  // B2 Middle Lane (Bottom vertical)
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5 - 20, SCREEN_HEIGHT + 40, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5 - 20;
                    newVehicle.targetY = SCREEN_HEIGHT * 2 / 3 + 50;
                    break;
                case 'C':  // C2 Middle Lane (Right horizontal)
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH + 40, SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5 - 20, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH * 2 / 3 + 50;
                    newVehicle.targetY = SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5 - 20;
                    break;
                case 'D':  // D2 Middle Lane (Left horizontal)
                    newVehicle.rect = (SDL_Rect){0 - 40, SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5 - 20, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 3 - 50;
                    newVehicle.targetY = SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5 - 20;
                    break;
            }
            
            enqueueVehicle(&queues[i], newVehicle);
            queues[i].lastGenerationTime = currentTime;
        }
    }
}

// Function to generate vehicles for incoming lanes
void generateVehicles(VehicleQueue* queues, int queueCount) {
    Uint32 currentTime = SDL_GetTicks();
    
    for (int i = 0; i < queueCount; i++) {
        if (currentTime - queues[i].lastGenerationTime >= queues[i].generationInterval) {
            // Generate vehicle based on which queue (road)
            Vehicle newVehicle;
            newVehicle.active = true;
            newVehicle.speed = 4;  // Default speed
            generateVehicleNumber(newVehicle.number);
            
            switch (i) {
                case 0:  // D3 to A1
                    newVehicle.rect = (SDL_Rect){0 - 40, SCREEN_HEIGHT / 3 + LANE_WIDTH / 3, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 3 + LANE_WIDTH / 4;
                    newVehicle.targetY = -40;
                    newVehicle.road = 'D';
                    newVehicle.lane = 3;
                    break;
                case 1:  // B3 to D1
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH / 3 + LANE_WIDTH / 4, SCREEN_HEIGHT + 40, 40, 40};
                    newVehicle.targetX = -40;
                    newVehicle.targetY = SCREEN_HEIGHT / 1.55;
                    newVehicle.road = 'B';
                    newVehicle.lane = 3;
                    break;
                case 2:  // C3 to B1
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH, SCREEN_HEIGHT / 3 + 2.4 * LANE_WIDTH, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 1.69;
                    newVehicle.targetY = SCREEN_HEIGHT;
                    newVehicle.road = 'C';
                    newVehicle.lane = 3;
                    break;
                case 3:  // A3 to C1
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH / 3 + 2.4 * LANE_WIDTH, 0, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH;
                    newVehicle.targetY = SCREEN_HEIGHT / 2.8;
                    newVehicle.road = 'A';
                    newVehicle.lane = 3;
                    break;
            }
            
            newVehicle.isPriority = false;  // Incoming lanes are not priority
            enqueueVehicle(&queues[i], newVehicle);
            queues[i].lastGenerationTime = currentTime;
        }
    }
}

// Function to draw individual lane divisions without crossing stop lines
void drawLaneDivisions(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, laneDivisionColor.r, laneDivisionColor.g, laneDivisionColor.b, laneDivisionColor.a);

    // Vertical lane divisions (dashed effect for each road)
    // North Road (Top) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 3 + i * LANE_WIDTH, 0, SCREEN_WIDTH / 3 + i * LANE_WIDTH, SCREEN_HEIGHT / 3 - 10);
    }

    // South Road (Bottom) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 3 + i * LANE_WIDTH, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH / 3 + i * LANE_WIDTH, SCREEN_HEIGHT);
    }

    // Horizontal lane divisions (dashed effect for each road)
    // West Road (Left) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT / 3 + i * LANE_WIDTH, SCREEN_WIDTH / 3 - 10, SCREEN_HEIGHT / 3 + i * LANE_WIDTH);
    }

    // East Road (Right) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT / 3 + i * LANE_WIDTH, SCREEN_WIDTH, SCREEN_HEIGHT / 3 + i * LANE_WIDTH);
    }
}

// Function to draw the roads
void drawCrossroad(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);  // Dark gray for roads

    // Draw vertical roads (A1, A2, A3)
    for (int i = 0; i < 3; i++) {
        SDL_Rect verticalRoad = {SCREEN_WIDTH / 3 + i * LANE_WIDTH, 0, LANE_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &verticalRoad);
    }

    // Draw horizontal roads (D1, D2, D3)
    for (int i = 0; i < 3; i++) {
        SDL_Rect horizontalRoad = {0, SCREEN_HEIGHT / 3 + i * LANE_WIDTH, SCREEN_WIDTH, LANE_WIDTH};
        SDL_RenderFillRect(renderer, &horizontalRoad);
    }

    // Draw lane divisions for each road (stopping before stop lines)
    drawLaneDivisions(renderer);
}

// Function to draw traffic lights for all middle lanes (A2, B2, C2, D2)
void drawTrafficLights(SDL_Renderer *renderer, TrafficLight lights[], int lightCount) {
    for (int i = 0; i < lightCount; i++) {
        // Set color based on traffic light state
        if (lights[i].state == RED) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green
        }
        
        drawCircle(renderer, lights[i].x, lights[i].y, lights[i].radius);
    }
}

// Function to render text (lane names)
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Function to draw the vehicle (simple rectangle for now)
void drawVehicle(SDL_Renderer *renderer, Vehicle *vehicle) {
    if (vehicle->active) {
        if (vehicle->isPriority) {
            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);  // Orange for priority vehicles
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color for the vehicle
        }
        SDL_RenderFillRect(renderer, &vehicle->rect);  // Draw the vehicle rectangle
    }
}

// Movement functions for incoming lanes
void moveVehicleD3toA1(Vehicle *vehicle, TrafficLight *a2Light) {
    // Check if at intersection and if the light is red
    bool atIntersection = (vehicle->rect.x >= SCREEN_WIDTH / 3 - vehicle->rect.w && 
                          vehicle->rect.y <= SCREEN_HEIGHT / 3 + LANE_WIDTH);
    
    if (atIntersection && a2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.x < vehicle->targetX) {
        vehicle->rect.x += vehicle->speed;  // Move right
    } else if (vehicle->rect.y > vehicle->targetY) {
        vehicle->rect.y -= vehicle->speed;  // Move up past A1
    }
}

void moveVehicleB3toD1(Vehicle *vehicle, TrafficLight *d2Light) {
    // Check if at intersection and if the light is red
    bool atIntersection = (vehicle->rect.y <= SCREEN_HEIGHT / 3 + 2 * LANE_WIDTH &&
                          vehicle->rect.x <= SCREEN_WIDTH / 3 + LANE_WIDTH);
    
    if (atIntersection && d2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.y > vehicle->targetY) {
        vehicle->rect.y -= vehicle->speed;  // Move up
    } else if (vehicle->rect.x > vehicle->targetX) {
        vehicle->rect.x -= vehicle->speed;  // Move left past D1
    }
}

void moveVehicleC3toB1(Vehicle *vehicle, TrafficLight *b2Light) {
    // Check if at intersection and if the light is red
    bool atIntersection = (vehicle->rect.x <= SCREEN_WIDTH * 2 / 3 + vehicle->rect.w &&
                          vehicle->rect.y >= SCREEN_HEIGHT / 3 - vehicle->rect.h);
    
    if (atIntersection && b2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.x > vehicle->targetX) {
        vehicle->rect.x -= vehicle->speed;  // Move west
    } else if (vehicle->rect.y < vehicle->targetY) {
        vehicle->rect.y += vehicle->speed;  // Move south
    }
}

void moveVehicleA3toC1(Vehicle *vehicle, TrafficLight *c2Light) {
    // Check if at intersection and if the light is red
    bool atIntersection = (vehicle->rect.y >= SCREEN_HEIGHT / 3 - vehicle->rect.h &&
                          vehicle->rect.x >= SCREEN_WIDTH / 3 + 2 * LANE_WIDTH);
    
    if (atIntersection && c2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.y < vehicle->targetY) {
        vehicle->rect.y += vehicle->speed;  // Move south
    } else if (vehicle->rect.x < vehicle->targetX) {
        vehicle->rect.x += vehicle->speed;  // Move east
    }
}

// Movement functions for middle lanes
void moveVehicleA2toB2(Vehicle *vehicle, TrafficLight *a2Light) {
    // Check if the light is red and vehicle is at intersection
    bool atIntersection = (vehicle->rect.y >= SCREEN_HEIGHT / 3 - vehicle->rect.h);
    
    if (atIntersection && a2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.y < SCREEN_HEIGHT / 2) {
        vehicle->rect.y += vehicle->speed;  // Move down
    } else if (vehicle->rect.y < SCREEN_HEIGHT) {
        vehicle->rect.y += vehicle->speed;  // Continue moving down
    }
}

void moveVehicleB2toA2(Vehicle *vehicle, TrafficLight *b2Light) {
    // Check if the light is red and vehicle is at intersection
    bool atIntersection = (vehicle->rect.y <= SCREEN_HEIGHT * 2 / 3);
    
    if (atIntersection && b2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.y > SCREEN_HEIGHT / 2) {
        vehicle->rect.y -= vehicle->speed;  // Move up
    } else if (vehicle->rect.y > 0) {
        vehicle->rect.y -= vehicle->speed;  // Continue moving up
    }
}

void moveVehicleC2toD2(Vehicle *vehicle, TrafficLight *c2Light) {
    // Check if the light is red and vehicle is at intersection
    bool atIntersection = (vehicle->rect.x <= SCREEN_WIDTH * 2 / 3);
    
    if (atIntersection && c2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.x > SCREEN_WIDTH / 2) {
        vehicle->rect.x -= vehicle->speed;  // Move left
    } else if (vehicle->rect.x > 0) {
        vehicle->rect.x -= vehicle->speed;  // Continue moving left
    }
}

void moveVehicleD2toC2(Vehicle *vehicle, TrafficLight *d2Light) {
    // Check if the light is red and vehicle is at intersection
    bool atIntersection = (vehicle->rect.x >= SCREEN_WIDTH / 3);
    
    if (atIntersection && d2Light->state == RED) {
        return;  // Stop at red light
    }
    
    if (vehicle->rect.x < SCREEN_WIDTH / 2) {
        vehicle->rect.x += vehicle->speed;  // Move right
    } else if (vehicle->rect.x < SCREEN_WIDTH) {
        vehicle->rect.x += vehicle->speed;  // Continue moving right
    }
}

// Check if vehicle has reached destination
bool hasReachedDestination(Vehicle* vehicle) {
    switch (vehicle->road) {
        case 'A':
            if (vehicle->lane == 2) {
                return vehicle->rect.y >= SCREEN_HEIGHT;
            } else if (vehicle->lane == 3) {
                return vehicle->rect.x >= SCREEN_WIDTH;
            }
            break;
        case 'B':
            if (vehicle->lane == 2) {
                return vehicle->rect.y <= 0;
            } else if (vehicle->lane == 3) {
                return vehicle->rect.x <= 0;
            }
            break;
        case 'C':
            if (vehicle->lane == 2) {
                return vehicle->rect.x <= 0;
            } else if (vehicle->lane == 3) {
                return vehicle->rect.y >= SCREEN_HEIGHT;
            }
            break;
        case 'D':
            if (vehicle->lane == 2) {
                return vehicle->rect.x >= SCREEN_WIDTH;
            } else if (vehicle->lane == 3) {
                return vehicle->rect.y <= 0;
            }
            break;
    }
    return false;
}

// Update priority status for A2 lane and set traffic lights accordingly
void updatePriorityStatus(VehicleQueue* a2Queue, TrafficLight* trafficLights) {
    // Check if A2 has more than 5 vehicles
    if (a2Queue->size > 5) {
        // Set A2 to green and all others to red
        trafficLights[0].state = GREEN;  // A2
        trafficLights[0].isPriority = true;
        trafficLights[1].state = RED;    // B2
        trafficLights[2].state = RED;    // C2
        trafficLights[3].state = RED;    // D2
    } else {
        // Reset priority flag
        trafficLights[0].isPriority = false;
    }
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;  // Prevent unused parameter warnings

    // Seed random number generator
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Traffic Simulation",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load font for lane names
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);  // Make sure you have this font
    if (!font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize traffic lights for middle lanes (A2, B2, C2, D2)
    // With different initial states and toggle durations
    TrafficLight trafficLights[4];
    trafficLights[0] = initTrafficLight(SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5, SCREEN_HEIGHT / 4, 15, RED, 5000);    // A2 light
    trafficLights[1] = initTrafficLight(SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5, SCREEN_HEIGHT * 3 / 4, 15, GREEN, 5000); // B2 light
    trafficLights[2] = initTrafficLight(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5, 15, RED, 5000); // C2 light
    trafficLights[3] = initTrafficLight(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5, 15, GREEN, 5000);  // D2 light

    // Initialize vehicle queues for each incoming lane
    const int MAX_VEHICLES = 10;
    VehicleQueue incomingVehicleQueues[4];
    
    // Different generation intervals for variety (milliseconds)
    incomingVehicleQueues[0] = initVehicleQueue(MAX_VEHICLES, 3000, 'D', 3);  // D3 to A1 vehicles
    incomingVehicleQueues[1] = initVehicleQueue(MAX_VEHICLES, 4000, 'B', 3);  // B3 to D1 vehicles
    incomingVehicleQueues[2] = initVehicleQueue(MAX_VEHICLES, 3500, 'C', 3);  // C3 to B1 vehicles
    incomingVehicleQueues[3] = initVehicleQueue(MAX_VEHICLES, 4500, 'A', 3);  // A3 to C1 vehicles

    // Initialize vehicle queues for middle lanes
    VehicleQueue middleLaneQueues[4];
    middleLaneQueues[0] = initVehicleQueue(MAX_VEHICLES, 2000, 'A', 2);  // A2 to B2 vehicles
    middleLaneQueues[1] = initVehicleQueue(MAX_VEHICLES, 2500, 'B', 2);  // B2 to A2 vehicles
    middleLaneQueues[2] = initVehicleQueue(MAX_VEHICLES, 3000, 'C', 2);  // C2 to D2 vehicles
    middleLaneQueues[3] = initVehicleQueue(MAX_VEHICLES, 3500, 'D', 2);  // D2 to C2 vehicles

    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);  // Green background
    SDL_RenderClear(renderer);

    drawCrossroad(renderer);
    drawTrafficLights(renderer, trafficLights, 4);

    SDL_Color laneColor = {255, 255, 255, 255};  // White text color

    int running = 1;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Check A2 priority status and update traffic lights accordingly
        updatePriorityStatus(&middleLaneQueues[0], trafficLights);
        
        // Only update non-priority traffic lights
        for (int i = 0; i < 4; i++) {
            if (!trafficLights[i].isPriority) {
                updateTrafficLight(&trafficLights[i]);
            }
        }
        
        // Generate new vehicles periodically
        generateVehicles(incomingVehicleQueues, 4);
        generateMiddleLaneVehicles(middleLaneQueues, 4);
        
        // Process incoming lane vehicles
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < incomingVehicleQueues[i].capacity; j++) {
                if (incomingVehicleQueues[i].vehicles[j].active) {
                    // Update vehicle positions based on queue type
                    switch (i) {
                        case 0:  // D3 to A1
                            moveVehicleD3toA1(&incomingVehicleQueues[i].vehicles[j], &trafficLights[0]);
                            break;
  case 1:  // B3 to D1
                            moveVehicleB3toD1(&incomingVehicleQueues[i].vehicles[j], &trafficLights[3]);
                            break;
                        case 2:  // C3 to B1
                            moveVehicleC3toB1(&incomingVehicleQueues[i].vehicles[j], &trafficLights[1]);
                            break;
                        case 3:  // A3 to C1
                            moveVehicleA3toC1(&incomingVehicleQueues[i].vehicles[j], &trafficLights[2]);
                            break;
                    }
                    
                    // Check if vehicle reached destination
                    if (hasReachedDestination(&incomingVehicleQueues[i].vehicles[j])) {
                        incomingVehicleQueues[i].vehicles[j].active = false;
                    }
                }
            }
        }
        
        // Process middle lane vehicles
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < middleLaneQueues[i].capacity; j++) {
                if (middleLaneQueues[i].vehicles[j].active) {
                    // Update vehicle positions based on lane
                    switch (i) {
                        case 0:  // A2 to B2
                            moveVehicleA2toB2(&middleLaneQueues[i].vehicles[j], &trafficLights[0]);
                            break;
                        case 1:  // B2 to A2
                            moveVehicleB2toA2(&middleLaneQueues[i].vehicles[j], &trafficLights[1]);
                            break;
                        case 2:  // C2 to D2
                            moveVehicleC2toD2(&middleLaneQueues[i].vehicles[j], &trafficLights[2]);
                            break;
                        case 3:  // D2 to C2
                            moveVehicleD2toC2(&middleLaneQueues[i].vehicles[j], &trafficLights[3]);
                            break;
                    }
                    
                    // Check if vehicle reached destination
                    if (hasReachedDestination(&middleLaneQueues[i].vehicles[j])) {
                        middleLaneQueues[i].vehicles[j].active = false;
                    }
                }
            }
        }
        
        // Dequeue vehicles from A2 if it has priority and light is green
        if (trafficLights[0].isPriority && trafficLights[0].state == GREEN) {
            // Find the frontmost vehicle in A2 queue
            int frontVehicleIndex = -1;
            int minY = SCREEN_HEIGHT;
            
            for (int j = 0; j < middleLaneQueues[0].capacity; j++) {
                if (middleLaneQueues[0].vehicles[j].active && 
                    middleLaneQueues[0].vehicles[j].rect.y < minY &&
                    middleLaneQueues[0].vehicles[j].rect.y >= SCREEN_HEIGHT / 3) {
                    minY = middleLaneQueues[0].vehicles[j].rect.y;
                    frontVehicleIndex = j;
                }
            }
            
            // Move the frontmost vehicle
            if (frontVehicleIndex != -1) {
                middleLaneQueues[0].vehicles[frontVehicleIndex].speed = 6;  // Slightly faster when dequeuing
            }
        }
        
        // Clear the renderer
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);  // Green background
        SDL_RenderClear(renderer);
        
        // Draw roads, traffic lights, and lane names
        drawCrossroad(renderer);
        drawTrafficLights(renderer, trafficLights, 4);
        
        // Draw text for lane names (A1, A2, etc.)
        SDL_Color laneColor = {255, 255, 255, 255};  // White text color
        
        // Draw lane names
        renderText(renderer, font, "A1", SCREEN_WIDTH / 3 + LANE_WIDTH / 4 - 15, SCREEN_HEIGHT / 6, laneColor);
        renderText(renderer, font, "A2", SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5 - 15, SCREEN_HEIGHT / 6, laneColor);
        renderText(renderer, font, "A3", SCREEN_WIDTH / 3 + LANE_WIDTH * 2.5 - 15, SCREEN_HEIGHT / 6, laneColor);
        
        renderText(renderer, font, "B1", SCREEN_WIDTH / 3 + LANE_WIDTH / 4 - 15, SCREEN_HEIGHT * 5 / 6, laneColor);
        renderText(renderer, font, "B2", SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5 - 15, SCREEN_HEIGHT * 5 / 6, laneColor);
        renderText(renderer, font, "B3", SCREEN_WIDTH / 3 + LANE_WIDTH * 2.5 - 15, SCREEN_HEIGHT * 5 / 6, laneColor);
        
        renderText(renderer, font, "C1", SCREEN_WIDTH * 5 / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH / 4 - 15, laneColor);
        renderText(renderer, font, "C2", SCREEN_WIDTH * 5 / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5 - 15, laneColor);
        renderText(renderer, font, "C3", SCREEN_WIDTH * 5 / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH * 2.5 - 15, laneColor);
        
        renderText(renderer, font, "D1", SCREEN_WIDTH / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH / 4 - 15, laneColor);
        renderText(renderer, font, "D2", SCREEN_WIDTH / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH * 1.5 - 15, laneColor);
        renderText(renderer, font, "D3", SCREEN_WIDTH / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH * 2.5 - 15, laneColor);
        
        // Draw all vehicles from incoming lanes
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < incomingVehicleQueues[i].capacity; j++) {
                if (incomingVehicleQueues[i].vehicles[j].active) {
                    drawVehicle(renderer, &incomingVehicleQueues[i].vehicles[j]);
                }
            }
        }
        
        // Draw all vehicles from middle lanes
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < middleLaneQueues[i].capacity; j++) {
                if (middleLaneQueues[i].vehicles[j].active) {
                    drawVehicle(renderer, &middleLaneQueues[i].vehicles[j]);
                }
            }
        }
        
        // Update queue size counts (for priority logic, but not displayed)
        int queueSizes[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < middleLaneQueues[i].capacity; j++) {
                if (middleLaneQueues[i].vehicles[j].active) {
                    queueSizes[i]++;
                }
            }
            
            // Update queue size in the structure
            middleLaneQueues[i].size = queueSizes[i];
        }
        
        SDL_RenderPresent(renderer);
        
        // Cap the frame rate to prevent CPU hogging
        SDL_Delay(16);  // ~60 FPS
    }
    
    // Clean up
    for (int i = 0; i < 4; i++) {
        free(incomingVehicleQueues[i].vehicles);
        free(middleLaneQueues[i].vehicles);
    }
    
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}



