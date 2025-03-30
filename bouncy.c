#include <stdio.h>
#include <windows.h>
#define SDL_MAIN_HANDLED
#include "SDL.h"

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_GRAY 0xf1f1f1f1
#define COLOR_TRAJECTORY 0xff763b
#define A_GRAVITY 0.2
#define DAMPENING 0.8
#define BG_COLOR 0x0f0f0f0f
#define TRAJECTORY_LENGTH 100
#define TRAJECTORY_WIDTH 10

#undef main

struct Circle {
    double x;
    double y;
    double radius;
    double v_x;
    double v_y;
};

void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color) {
    double low_x = circle.x - circle.radius;
    double low_y = circle.y - circle.radius;
    double high_x = circle.x + circle.radius;
    double high_y = circle.y + circle.radius;

    double radius_squared = circle.radius * circle.radius;

    for (double x = low_x; x < high_x; x++) {
        for (double y = low_y; y < high_y; y++) {
            // is coordinate within circle
            double center_dist_squared = (x - circle.x) * (x - circle.x) + (y - circle.y) * (y - circle.y);
            if (center_dist_squared < radius_squared) {
                SDL_Rect pixel = (SDL_Rect) {(int)x, (int)y, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }   
        }
    }
}

void FillTrajectory(SDL_Surface* surface, struct Circle trajectory[TRAJECTORY_LENGTH], int current_trajectory_index) {
    for (int i = 0; i < current_trajectory_index; i++) {
        double trajectory_size = TRAJECTORY_WIDTH * (double) i / 100;
        trajectory[i].radius = trajectory_size;
        FillCircle(surface, trajectory[i], COLOR_TRAJECTORY);
    }
}

void step(struct Circle* circle) {
    circle->x += circle->v_x;
    circle->y += circle->v_y;
    circle->v_y += A_GRAVITY;

    if (circle->x + circle->radius > WIDTH) {
        circle->x = WIDTH - circle->radius;
        circle->v_x = -circle->v_x * DAMPENING;
    }
    if (circle->y + circle->radius > HEIGHT) {
        circle->y = HEIGHT - circle->radius;
        circle->v_y = -circle->v_y * DAMPENING;
    }
    if (circle->y - circle->radius < 0) {
        circle->y = circle->radius;
        circle->v_y = -circle->v_y * DAMPENING;
    }
    if (circle->x - circle->radius < 0) {
        circle->x = circle->radius;
        circle->v_x = -circle->v_x * DAMPENING;
    }
}

void UpdateTrajectory(struct Circle trajectory[TRAJECTORY_LENGTH], struct Circle circle, int current_index) {
    if (current_index >= TRAJECTORY_LENGTH) {
        // shift array - write the circle at the end of the array
        for (int i = 0; i < TRAJECTORY_LENGTH - 1; i++) {
            trajectory[i] = trajectory[i + 1];
        }
        trajectory[TRAJECTORY_LENGTH - 1] = circle;
    } else {
        trajectory[current_index] = circle;
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    if (hIcon) {
        MessageBox(NULL, "Icon loaded successfully!", "Info", MB_OK);
    } else {
        MessageBox(NULL, "Failed to load icon", "Error", MB_OK | MB_ICONERROR);
    }

    char debugMessage[256];
    snprintf(debugMessage, sizeof(debugMessage), "Command Line arguments: %s\n", lpCmdLine);
    OutputDebugString(debugMessage);

    if (nShowCmd == SW_SHOWMAXIMIZED) {
        MessageBox(NULL, "WIndow will be maximized", "Info", MB_OK);
    } else if (nShowCmd == SW_SHOWMINIMIZED) {
        MessageBox(NULL, "Window will be minimized", "Info", MB_OK);
    } else {
        MessageBox(NULL, "Window will be shown normally", "Info", MB_OK);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        MessageBox(NULL, "SDL Initialization failed", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Bouncy Ball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS);
    if (!window) {
        MessageBox(NULL, "Window creation failed", "Error", MB_OK | MB_ICONERROR);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        MessageBox(NULL, "Failed to get window surface", "Error", MB_OK | MB_ICONERROR);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    struct Circle circle = {200, 200, 80, 50, 50};
    struct Circle trajectory[TRAJECTORY_LENGTH];
    int current_trajectory_index = 0;

    SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};
    SDL_Event event;
    int simulation_running = 1;
    while (simulation_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) 
                simulation_running = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    simulation_running = 0;
                }
            }
        }

        SDL_FillRect(surface, &erase_rect, BG_COLOR);
        FillTrajectory(surface, trajectory, current_trajectory_index);
        FillCircle(surface, circle, COLOR_WHITE);
        step(&circle);
        UpdateTrajectory(trajectory, circle, current_trajectory_index);
        if (current_trajectory_index < TRAJECTORY_LENGTH) 
            ++current_trajectory_index;
        SDL_UpdateWindowSurface(window);
        SDL_Delay(20);
    }  

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}