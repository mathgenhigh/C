#include <stdio.h>
#include <windows.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include "SDL.h"

#define WIDTH 1200
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_RAY 0xffd43b
#define COLOR_RAY_BLUR 0xbd6800
#define RAYS_NUMBER 500
#define RAY_THICKNESS 1

#undef main

struct Circle {
    double x;
    double y;
    double radius;
};

struct Ray {
    double x_start, y_start;
    double angle;
};

void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color) {
    double radius_squared = pow(circle.radius, 2); 
    for (double x = circle.x - circle.radius; x <= circle.x + circle.radius; x++) {
        for (double y = circle.y - circle.radius; y <= circle.y + circle.radius; y++) {
            double dist_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);
            if (dist_squared < radius_squared) {
                SDL_Rect pixel = (SDL_Rect) {x, y, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        double angle = ((double) i / RAYS_NUMBER) * 2 * M_PI; 
        struct Ray ray = {circle.x, circle.y, angle};
        rays[i] = ray;
    }
}

void FillRays(SDL_Surface* surface, struct Ray rays[RAYS_NUMBER], Uint32 color, Uint32 blur_color, struct Circle object) {

    double radius_squared = pow(object.radius, 2);
    for (int i = 0; i < RAYS_NUMBER; i++) {
        struct Ray ray = rays[i];

        int end_of_screen = 0;
        int object_hit = 0;

        double step = 1;
        double x_draw = ray.x_start;
        double y_draw = ray.y_start;
        while (!end_of_screen && !object_hit) {
            x_draw += step * cos(ray.angle);
            y_draw += step * sin(ray.angle);  

            SDL_Rect ray_point = (SDL_Rect) {x_draw, y_draw, RAY_THICKNESS, RAY_THICKNESS};
            double blur_point = 3 * RAY_THICKNESS;
            SDL_Rect ray_blur = (SDL_Rect) {x_draw, y_draw, blur_point, blur_point};
            SDL_FillRect(surface, &ray_point, color);

            if (x_draw < 0 || x_draw > WIDTH) end_of_screen = 1;
            if (y_draw < 0 || y_draw > HEIGHT) end_of_screen = 1;
            
            double dist_squared = pow(x_draw - object.x, 2) + pow(y_draw - object.y, 2);
            if (dist_squared < radius_squared) {
                break;
            }
        }
    }
}

void PrintWinEvent(const SDL_Event* event) {
    if(event->type == SDL_WINDOWEVENT) {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
          SDL_Log("Window %d shown", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_HIDDEN:
          SDL_Log("Window %d hidden", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_EXPOSED:
          SDL_Log("Window %d exposed", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_MOVED:
          SDL_Log("Window %d moved to %d, %d", event->window.windowID, event->window.data1, event->window.data2);
          break;
        case SDL_WINDOWEVENT_RESIZED:
          SDL_Log("Window %d resized to %dx%d", event->window.windowID, event->window.data1, event->window.data2);
          break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          SDL_Log("Window %d size changed to %dx%d", event->window.windowID, event->window.data1, event->window.data2);
          break;
        case SDL_WINDOWEVENT_MINIMIZED:
          SDL_Log("Window %d minimized", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_MAXIMIZED:
          SDL_Log("Window %d maximized", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_RESTORED:
          SDL_Log("Window %d restored", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_ENTER:
          SDL_Log("Mouse entered window %d", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_LEAVE:
          SDL_Log("Mouse left window %d", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
          SDL_Log("Window %d gained keyboard focus", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
          SDL_Log("Window %d lost keyboard focus", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_CLOSE:
          SDL_Log("Window %d closed", event->window.windowID);
          break;
  #if SDL_VERSION_ATLEAST(2, 0, 5) 
        case SDL_WINDOWEVENT_TAKE_FOCUS:
          SDL_Log("Window %d is offered a focus", event->window.windowID);
          break;
        case SDL_WINDOWEVENT_HIT_TEST:
          SDL_Log("Window %d has a special hit test", event->window.windowID);
          break;
  #endif
        default:
          SDL_Log("Window %d got unknown event %d", event->window.windowID, event->window.windowID);
          break;
        }
    }
}

int APIENTRY WinMain(HINSTANCE hIsntance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        MessageBox(NULL, "SDL initialization failed.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (!window) {
        MessageBox(NULL, "Window creation failed", "Error", MB_OK | MB_ICONERROR);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        MessageBox(NULL, "Failed to load window surface", "Error", MB_OK | MB_ICONERROR);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    struct Circle circle = {200, 200, 40};
    struct Circle shadow_circle = {600, 300, 140};
    SDL_Rect erase_rect = (SDL_Rect) {0, 0, WIDTH, HEIGHT};
    struct Ray rays[RAYS_NUMBER];
    generate_rays(circle, rays);
    double obstacle_speed_y = 4;
    
    int is_running = 1;
    while (is_running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch(ev.type) {
                case SDL_MOUSEBUTTONDOWN:
                    printf("mouse\n");
                    break;
                case SDL_QUIT:
                    printf("quit\n");
                    is_running = 0;
                    break;
                case SDL_MOUSEMOTION:
                    if (ev.motion.state != 0) {
                        circle.x = ev.motion.x;
                        circle.y = ev.motion.y;
                        generate_rays(circle, rays);
                    }
                    break;
            }
            PrintWinEvent(&ev);
        }

        SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
        FillRays(surface, rays, COLOR_RAY, COLOR_RAY_BLUR, shadow_circle);
        FillCircle(surface, circle, COLOR_WHITE);
        FillCircle(surface, shadow_circle, COLOR_WHITE);
        shadow_circle.y += obstacle_speed_y;
        if (shadow_circle.y - shadow_circle.radius < 0) obstacle_speed_y = -obstacle_speed_y;
        if (shadow_circle.y + shadow_circle.radius > HEIGHT) obstacle_speed_y = -obstacle_speed_y;
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}