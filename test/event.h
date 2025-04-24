#ifndef EVENT_H
#define EVENT_H

#include <SDL.h>
#include <vector>
#include "bullet.h"
#include <cmath>

struct Player {
    SDL_Rect rect;
    double angle = 0;
    float posX, posY;
    int velX = 0, velY = 0;
    bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;
    float fireTimer = 0;
    float health = 1.0f;

    Player() {
        rect = { 600, 300, 64, 64 };
        posX = rect.x;
        posY = rect.y;
    }
};

void handleEvent(SDL_Event& event, bool& running, Player& player, std::vector<Bullet>& bullets) {
    if (event.type == SDL_QUIT) {
        running = false;
    }
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_w: player.moveUp = true; break;
        case SDLK_s: player.moveDown = true; break;
        case SDLK_a: player.moveLeft = true; break;
        case SDLK_d: player.moveRight = true; break;
        case SDLK_ESCAPE: running = false; break;
        }
    }
    if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
        case SDLK_w: player.moveUp = false; break;
        case SDLK_s: player.moveDown = false; break;
        case SDLK_a: player.moveLeft = false; break;
        case SDLK_d: player.moveRight = false; break;
        }
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (player.fireTimer <= 0) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                float dx = mouseX - (player.posX + player.rect.w / 2);
                float dy = mouseY - (player.posY + player.rect.h / 2);
                float length = std::sqrt(dx * dx + dy * dy);
                if (length != 0) {
                    dx /= length;
                    dy /= length;
                }
                Bullet b(player.posX + player.rect.w / 2, player.posY + player.rect.h / 2, dx, dy, 700);
                b.active = true;
                bullets.push_back(b);
                player.fireTimer = 0.1f;
            }
        }
    }
}

void updatePlayer(Player& player, float deltaTime, int windowWidth, int windowHeight, std::vector<Bullet>& bullets) {
    player.velX = 0;
    player.velY = 0;
    float speed = 500.0f;

    if (player.moveUp) player.velY = -speed;
    if (player.moveDown) player.velY = speed;
    if (player.moveLeft) player.velX = -speed;
    if (player.moveRight) player.velX = speed;

    if (player.velX != 0 && player.velY != 0) {
        player.velX *= 0.7071f;
        player.velY *= 0.7071f;
    }

    player.posX += player.velX * deltaTime;
    player.posY += player.velY * deltaTime;

    if (player.posX < 0) player.posX = 0;
    if (player.posX + player.rect.w > windowWidth) player.posX = windowWidth - player.rect.w;
    if (player.posY < 0) player.posY = 0;
    if (player.posY + player.rect.h > windowHeight) player.posY = windowHeight - player.rect.h;

    player.rect.x = static_cast<int>(player.posX);
    player.rect.y = static_cast<int>(player.posY);

    if (player.fireTimer > 0) {
        player.fireTimer -= deltaTime;
        if (player.fireTimer < 0) player.fireTimer = 0;
    }
}

void updateRotation(Player& player, int mouseX, int mouseY) {
    float dx = mouseX - (player.posX + player.rect.w / 2);
    float dy = mouseY - (player.posY + player.rect.h / 2);
    player.angle = std::atan2(dy, dx) * 180.0f / M_PI + 90.0f;
}

#endif