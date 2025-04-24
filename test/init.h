#ifndef INIT_H
#define INIT_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

bool initGame(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Khong the khoi tao SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cout << "Khong the khoi tao SDL_image: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    if (TTF_Init() < 0) {
        std::cout << "Khong the khoi tao SDL_ttf: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    *window = SDL_CreateWindow("Space Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!*window) {
        std::cout << "Khong the tao cua so: " << SDL_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        std::cout << "Khong the tao renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(*window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    return true;
}

void cleanUp(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

#endif