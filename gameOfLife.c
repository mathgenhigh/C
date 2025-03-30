#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#define SDL_MAIN_HANDLED
#include "SDL.h"

#define WIDTH 900
#define HEIGHT 600
#define CELL_WIDTH 10
#define ROWS (HEIGHT / CELL_WIDTH)
#define COLS (WIDTH / CELL_WIDTH)

#define FRAME_DELAY 100

#define ALIVE 1
#define DEAD 0

#undef main

void draw_grid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 47, 47, 47, 255);
    for (int i = 0; i <= ROWS; i++) {
        SDL_RenderDrawLine(renderer, 0, i * CELL_WIDTH, WIDTH, i * CELL_WIDTH);
    }
    for (int j = 0; j <= COLS; j++) {
        SDL_RenderDrawLine(renderer, j * CELL_WIDTH, 0, j * CELL_WIDTH, HEIGHT);
    }
}

void render_game_matrix(SDL_Renderer* renderer, int* grid) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (grid[i * COLS + j] == ALIVE) {  // Only render alive cells
                SDL_Rect cell = {j * CELL_WIDTH, i * CELL_WIDTH, CELL_WIDTH, CELL_WIDTH};
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
}

int count_neighbors(int i, int j, int* grid) {
    int count = 0;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue;
            int ni = i + x, nj = j + y;
            if (ni >= 0 && ni < ROWS && nj >= 0 && nj < COLS) {
                count += grid[ni * COLS + nj];
            }
        }
    }
    return count;
}

void handle_mouse_click(int* grid, int x, int y) {
    int cell_x = x / CELL_WIDTH;
    int cell_y = y / CELL_WIDTH;
    int index = cell_y * COLS + cell_x;
    grid[index] ^= 1;
}

void save_pattern(const char* filename, int* grid) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        MessageBox(NULL, "Failed to save pattern!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            fprintf(file, "%d", grid[i * COLS + j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void load_pattern(const char* filename, int* grid) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        MessageBox(NULL, "Failed to load pattern", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            char c = fgetc(file);
            if (c == '1') {
                grid[i * COLS + j] = 1;
            } else if (c == '0') {
                grid[i * COLS + j] = 0;
            }
        }
        fgetc(file);
    }
    fclose(file);
}

void simulation_step(int* grid, int* buffer) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int neighbors = count_neighbors(i, j, grid);
            int index = i * COLS + j;
            buffer[index] = (grid[index] == ALIVE) ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
        }
    }
    memcpy(grid, buffer, ROWS * COLS * sizeof(int));
}

int main() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        MessageBox(NULL, "SDL initialization failed", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        MessageBox(NULL, "Window creation failed", "Error", MB_OK | MB_ICONERROR);
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        MessageBox(NULL, "Renderer creation failed", "Error", MB_OK | MB_ICONERROR);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int* grid = (int*)malloc(ROWS * COLS * sizeof(int));
    int* buffer = (int*)calloc(ROWS * COLS, sizeof(int)); 
    if (!grid || !buffer) {
        MessageBox(NULL, "Memory allocation failed", "Error", MB_OK | MB_ICONERROR);
        free(grid); free(buffer);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    memset(grid, 0, ROWS * COLS * sizeof(int));

    int running = 1, paused = 1;  
    SDL_Event event;
    Uint32 last_frame_time = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;  
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN:  
                        paused = 0;
                        break;
                    case SDLK_SPACE:   
                        paused = !paused;
                        break;
                    case SDLK_r:
                        memset(grid, 0, ROWS * COLS * sizeof(int));
                        paused = 1;  
                        break;
                    case SDLK_s:
                        save_pattern("pattern.txt", grid);
                        break;
                    case SDLK_l:
                        load_pattern("pattern.txt", grid);
                        break;
                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && paused) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                handle_mouse_click(grid, x, y); 
            }
        }

        if (!paused) {
            Uint32 current_time = SDL_GetTicks();
            if (current_time - last_frame_time >= FRAME_DELAY) {
                simulation_step(grid, buffer);  //
                last_frame_time = current_time;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);

        render_game_matrix(renderer, grid);
        draw_grid(renderer);

        SDL_RenderPresent(renderer);
    }

    free(grid);
    free(buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
