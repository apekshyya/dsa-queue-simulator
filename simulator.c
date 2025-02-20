#define SDL_MAIN_HANDLED  // Prevent SDL from overriding main
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>

const int SCREEN_WIDTH = 1000;  // Increased screen width (wider screen)
const int SCREEN_HEIGHT = 800;  // Keeping the same height

// Lane width for each lane
const int LANE_WIDTH = SCREEN_WIDTH / 9;

// Lane division colors (light and subtle)
SDL_Color laneDivisionColor = {180, 180, 180, 255};  // Light gray for subtle divisions

// Vehicle structure to hold properties of a vehicle
typedef struct {
    SDL_Rect rect;  // Rectangle for the vehicle (position and size)
    int speed;      // Speed of the vehicle
    int targetX;    // Target X position (destination)
    int targetY;    // Target Y position (destination)
} Vehicle;

// Declare the drawCircle function
void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius) {
    // Draw a circle using the midpoint circle algorithm
    for (int w = 0; w < 360; w++) {
        int x = (int)(centerX + radius * cos(w * M_PI / 180.0));
        int y = (int)(centerY + radius * sin(w * M_PI / 180.0));
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

// Function to draw individual lane divisions without crossing stop lines
void drawLaneDivisions(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, laneDivisionColor.r, laneDivisionColor.g, laneDivisionColor.b, laneDivisionColor.a);

    // Vertical lane divisions (dashed effect for each road)
    // North Road (Top) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 3 + i * LANE_WIDTH, 0, SCREEN_WIDTH / 3 + i * LANE_WIDTH, SCREEN_HEIGHT / 3 - 10); // Stop before stop line
    }

    // South Road (Bottom) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 3 + i * LANE_WIDTH, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH / 3 + i * LANE_WIDTH, SCREEN_HEIGHT); // Stop before stop line
    }

    // Horizontal lane divisions (dashed effect for each road)
    // West Road (Left) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT / 3 + i * LANE_WIDTH, SCREEN_WIDTH / 3 - 10, SCREEN_HEIGHT / 3 + i * LANE_WIDTH); // Stop before stop line
    }

    // East Road (Right) - Stop before stop lines
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT / 3 + i * LANE_WIDTH, SCREEN_WIDTH, SCREEN_HEIGHT / 3 + i * LANE_WIDTH); // Stop before stop line
    }
}

// Function to draw the roads
void drawCrossroad(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);  // Dark gray for roads

    // Draw vertical roads (A1, A2, A3)
    for (int i = 0; i < 3; i++) {
        SDL_Rect verticalRoad = {SCREEN_WIDTH / 3 + i * LANE_WIDTH, 0, LANE_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &verticalRoad);  // AL1, AL2, AL3 for vertical roads
    }

    // Draw horizontal roads (D1, D2, D3)
    for (int i = 0; i < 3; i++) {
        SDL_Rect horizontalRoad = {0, SCREEN_HEIGHT / 3 + i * LANE_WIDTH, SCREEN_WIDTH, LANE_WIDTH};
        SDL_RenderFillRect(renderer, &horizontalRoad);  // D1, D2, D3 for horizontal roads
    }

    // Draw lane divisions for each road (stopping before stop lines)
    drawLaneDivisions(renderer);
}

// Function to draw traffic lights at each road's intersection
void drawTrafficLights(SDL_Renderer *renderer) {
    int lightRadius = 15;  // Reduced radius of the traffic lights

    // Draw Traffic Light for North Road (Top)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color (initial state)
    drawCircle(renderer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2.6, lightRadius);  // North light

    // Draw Traffic Light for East Road (Right)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color (initial state)
    drawCircle(renderer, 5 * SCREEN_WIDTH / 8.1, SCREEN_HEIGHT / 1.85, lightRadius);  // East light

    // Draw Traffic Light for South Road (Bottom)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color (initial state)
    drawCircle(renderer, SCREEN_WIDTH / 2, 5 * SCREEN_HEIGHT / 8.2, lightRadius);  // South light

    // Draw Traffic Light for West Road (Left)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color (initial state)
    drawCircle(renderer, SCREEN_WIDTH / 2.8, SCREEN_HEIGHT / 1.8, lightRadius);  // West light
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
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color for the vehicle
    SDL_RenderFillRect(renderer, &vehicle->rect);  // Draw the vehicle rectangle
}

void moveVehicleD3toA1(Vehicle *vehicle) {
    if (vehicle->rect.x < vehicle->targetX) {
        vehicle->rect.x += vehicle->speed;  // Move right
    } else if (vehicle->rect.y > vehicle->targetY) {
        vehicle->rect.y -= vehicle->speed;  // Move up past A1
    }
}

// Function to move vehicle from B3 to D1
// 1. Moves north (decreasing Y) until aligned with D1
// 2. Moves west (decreasing X) to reach D1
void moveVehicleB3toD1(Vehicle *vehicle) {
    if (vehicle->rect.y > vehicle->targetY) {
        vehicle->rect.y -= vehicle->speed;  // Move up
    } else if (vehicle->rect.x > vehicle->targetX) {
        vehicle->rect.x -= vehicle->speed;  // Move left past D1
    }
}

void moveVehicleC3toB1(Vehicle *vehicle) {
    if (vehicle->rect.x > vehicle->targetX) {
        vehicle->rect.x -= vehicle->speed;  // Move west
    } else if (vehicle->rect.y < vehicle->targetY) {
        vehicle->rect.y += vehicle->speed;  // Move south
    }
}


void moveVehicleA3toC1(Vehicle *vehicle) {
    if (vehicle->rect.y < vehicle->targetY) {
        vehicle->rect.y += vehicle->speed;  // Move south
    } else if (vehicle->rect.x < vehicle->targetX) {
        vehicle->rect.x += vehicle->speed;  // Move east
    }
}


int main(int argc, char *argv[]) {
    (void)argc; (void)argv;  // Prevent unused parameter warnings

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

    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);  // Green background
    SDL_RenderClear(renderer);

    drawCrossroad(renderer);
    drawTrafficLights(renderer);

    // Render all lane names
    SDL_Color laneColor = {255, 255, 255, 255};  // White text color

  
    // Create vehicle instance D3 to A1
Vehicle vehicle1 = {
    {0 - 40, SCREEN_HEIGHT / 3 + LANE_WIDTH / 3, 40, 40},  // Start at the far west of D3
    4,  // Speed
    SCREEN_WIDTH / 3 + LANE_WIDTH / 4,  // Target X (A1)
    -40  // Target Y (Past A1 to the north end)
};

// Vehicle moving from B3 to D1
// Starting position at B3
// Target position at D1 (aligned with D1's coordinates)
// Vehicle moving from B3 to D1

Vehicle vehicle2 = {
    {SCREEN_WIDTH / 3 + LANE_WIDTH / 4, SCREEN_HEIGHT + 40, 40, 40},  // Start at the far south of B3
    4,  // Speed
    -40,  // Target X (Past D1 to the west end)
    SCREEN_HEIGHT / 1.55  // Target Y (D1)
};
  // Speed 4 pixels per frame

// C3 to B1: Start from east end, move west, then south
Vehicle vehicle3 = {
    {SCREEN_WIDTH, SCREEN_HEIGHT / 3 + 2.4 * LANE_WIDTH, 40, 40},  // Start at C3 (east end)
    4,  // Speed
    SCREEN_WIDTH / 1.69,  // Target X position (B1)
    SCREEN_HEIGHT  // Target Y position (south end)
};

// A3 to C1: Start from north end, move south, then east
Vehicle vehicle4 = {
    {SCREEN_WIDTH / 3 + 2.4 * LANE_WIDTH, 0, 40, 40},  // Start at A3 (north end)
    4,  // Speed
    SCREEN_WIDTH ,  // Target X position (C1)
    SCREEN_HEIGHT / 2.8  // Target Y position (south alignment with C1)
};

  // Speed 4 pixels per frame
    int running = 1;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

       // Move the vehicle and redraw
        // Move both vehicles simultaneously
moveVehicleD3toA1(&vehicle1);  // Move D3 → A1
moveVehicleB3toD1(&vehicle2);  // Move B3 → D1
moveVehicleC3toB1(&vehicle3);  // Move the vehicle from C3 to B1
moveVehicleA3toC1(&vehicle4);  // Move the vehicle from A3 to C1


        // Clear screen and redraw everything
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);  // Green background
        SDL_RenderClear(renderer);

        drawCrossroad(renderer);  // Draw the roads
        drawTrafficLights(renderer);  // Draw traffic lights

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

        // Draw both vehicles on the screen
        drawVehicle(renderer, &vehicle1);  // Draw D3 → A1 vehicle
        drawVehicle(renderer, &vehicle2);  // Draw B3 → D1 vehicle
        drawVehicle(renderer, &vehicle3);  // Draw the vehicle moving from C3 to B1
        drawVehicle(renderer, &vehicle4);  // Draw the vehicle moving from A3 to C1


        SDL_RenderPresent(renderer);

        SDL_Delay(30);  // Slow down the loop for smooth animation
    }

    TTF_CloseFont(font);  // Close the font
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
