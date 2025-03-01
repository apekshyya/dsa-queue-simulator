#define SDL_MAIN_HANDLED  // Prevent SDL from overriding main
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

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
} TrafficLight;

// Vehicle structure to hold properties of a vehicle
typedef struct {
    SDL_Rect rect;  // Rectangle for the vehicle (position and size)
    int speed;      // Speed of the vehicle
    int targetX;    // Target X position (destination)
    int targetY;    // Target Y position (destination)
    bool active;    // Whether vehicle is active/visible
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
VehicleQueue initVehicleQueue(int capacity, Uint32 generationInterval) {
    VehicleQueue queue;
    queue.capacity = capacity;
    queue.vehicles = (Vehicle*)malloc(capacity * sizeof(Vehicle));
    queue.size = 0;
    queue.front = 0;
    queue.rear = -1;
    queue.lastGenerationTime = SDL_GetTicks();
    queue.generationInterval = generationInterval;
    
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

// Function to generate vehicles periodically
void generateVehicles(VehicleQueue* queues, int queueCount) {
    Uint32 currentTime = SDL_GetTicks();
    
    for (int i = 0; i < queueCount; i++) {
        if (currentTime - queues[i].lastGenerationTime >= queues[i].generationInterval) {
            // Generate vehicle based on which queue (road)
            Vehicle newVehicle;
            newVehicle.active = true;
            newVehicle.speed = 4;  // Default speed
            
            switch (i) {
                case 0:  // D3 to A1
                    newVehicle.rect = (SDL_Rect){0 - 40, SCREEN_HEIGHT / 3 + LANE_WIDTH / 3, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 3 + LANE_WIDTH / 4;
                    newVehicle.targetY = -40;
                    break;
                case 1:  // B3 to D1
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH / 3 + LANE_WIDTH / 4, SCREEN_HEIGHT + 40, 40, 40};
                    newVehicle.targetX = -40;
                    newVehicle.targetY = SCREEN_HEIGHT / 1.55;
                    break;
                case 2:  // C3 to B1
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH, SCREEN_HEIGHT / 3 + 2.4 * LANE_WIDTH, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH / 1.69;
                    newVehicle.targetY = SCREEN_HEIGHT;
                    break;
                case 3:  // A3 to C1
                    newVehicle.rect = (SDL_Rect){SCREEN_WIDTH / 3 + 2.4 * LANE_WIDTH, 0, 40, 40};
                    newVehicle.targetX = SCREEN_WIDTH;
                    newVehicle.targetY = SCREEN_HEIGHT / 2.8;
                    break;
            }
            
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
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color for the vehicle
        SDL_RenderFillRect(renderer, &vehicle->rect);  // Draw the vehicle rectangle
    }
}

// Movement functions
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
    VehicleQueue vehicleQueues[4];
    
    // Different generation intervals for variety (milliseconds)
    vehicleQueues[0] = initVehicleQueue(MAX_VEHICLES, 3000);  // D3 to A1 vehicles
    vehicleQueues[1] = initVehicleQueue(MAX_VEHICLES, 4000);  // B3 to D1 vehicles
    vehicleQueues[2] = initVehicleQueue(MAX_VEHICLES, 3500);  // C3 to B1 vehicles
    vehicleQueues[3] = initVehicleQueue(MAX_VEHICLES, 4500);  // A3 to C1 vehicles

    // Active vehicles for display and movement
    Vehicle activeVehicles[MAX_VEHICLES * 4];
    int activeVehicleCount = 0;

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
        
        // Update traffic lights
        for (int i = 0; i < 4; i++) {
            updateTrafficLight(&trafficLights[i]);
        }
        
        // Generate new vehicles periodically
        generateVehicles(vehicleQueues, 4);
        
        // Process vehicle queues and update active vehicles
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < vehicleQueues[i].capacity; j++) {
                if (vehicleQueues[i].vehicles[j].active) {
                    // Update vehicle positions based on queue type
                    switch (i) {
                        case 0:  // D3 to A1
                            moveVehicleD3toA1(&vehicleQueues[i].vehicles[j], &trafficLights[0]);
                            break;
                        case 1:  // B3 to D1
                            moveVehicleB3toD1(&vehicleQueues[i].vehicles[j], &trafficLights[3]);
                            break;
                        case 2:  // C3 to B1
                            moveVehicleC3toB1(&vehicleQueues[i].vehicles[j], &trafficLights[1]);
                            break;
                        case 3:  // A3 to C1
                            moveVehicleA3toC1(&vehicleQueues[i].vehicles[j], &trafficLights[2]);
                            break;
                    }
                    
                    // Check if vehicle has reached its destination
                    Vehicle* v = &vehicleQueues[i].vehicles[j];
                    bool reachedDestination = false;
                    
                    switch (i) {
                        case 0:  // D3 to A1
                            reachedDestination = (v->rect.y <= v->targetY);
                            break;
                        case 1:  // B3 to D1
                            reachedDestination = (v->rect.x <= v->targetX);
                            break;
                        case 2:  // C3 to B1
                            reachedDestination = (v->rect.y >= v->targetY);
                            break;
                        case 3:  // A3 to C1
                            reachedDestination = (v->rect.x >= v->targetX);
                            break;
                    }
                    
                    if (reachedDestination) {
                        v->active = false;
                    }
                }
            }
        }

        // Clear screen and redraw everything
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);  // Green background
        SDL_RenderClear(renderer);

        drawCrossroad(renderer);  // Draw the roads
        drawTrafficLights(renderer, trafficLights, 4);  // Draw traffic lights

        // Render lane names
        renderText(renderer, font, "A1", SCREEN_WIDTH / 3 + LANE_WIDTH / 4, SCREEN_HEIGHT / 6, laneColor);
        renderText(renderer, font, "A2", SCREEN_WIDTH / 3 + LANE_WIDTH * 1.5, SCREEN_HEIGHT / 6, laneColor);
        renderText(renderer, font, "A3", SCREEN_WIDTH / 3 + LANE_WIDTH * 2.4, SCREEN_HEIGHT / 6, laneColor);
        renderText(renderer, font, "B3", SCREEN_WIDTH / 3 + LANE_WIDTH / 4, SCREEN_HEIGHT * 2 / 2.3, laneColor);
        renderText(renderer, font, "B2", SCREEN_WIDTH / 2.25 + LANE_WIDTH / 4, SCREEN_HEIGHT * 2 / 2.3, laneColor);
        renderText(renderer, font, "B1", SCREEN_WIDTH / 1.75 + LANE_WIDTH / 4, SCREEN_HEIGHT * 2 / 2.3, laneColor);
        renderText(renderer, font, "C1", SCREEN_WIDTH * 2 / 2.6, SCREEN_HEIGHT / 3 + LANE_WIDTH / 3, laneColor);
        renderText(renderer, font, "C2", SCREEN_WIDTH * 2 / 2.6, SCREEN_HEIGHT / 2.1 + LANE_WIDTH / 3, laneColor);
        renderText(renderer, font, "C3", SCREEN_WIDTH * 2 / 2.6, SCREEN_HEIGHT / 1.6 + LANE_WIDTH / 3, laneColor);
        renderText(renderer, font, "D3", SCREEN_WIDTH / 6, SCREEN_HEIGHT / 3 + LANE_WIDTH / 3, laneColor);
        renderText(renderer, font, "D2", SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2.1 + LANE_WIDTH / 3, laneColor);
        renderText(renderer, font, "D1", SCREEN_WIDTH / 6, SCREEN_HEIGHT / 1.66 + LANE_WIDTH / 3, laneColor);

        // Draw all active vehicles
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < vehicleQueues[i].capacity; j++) {
                if (vehicleQueues[i].vehicles[j].active) {
                    drawVehicle(renderer, &vehicleQueues[i].vehicles[j]);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(30);  // Slow down the loop for smooth animation
    }

    // Clean up
    for (int i = 0; i < 4; i++) {
        free(vehicleQueues[i].vehicles);
    }
    
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
