#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>
#include "init.h"

struct Bullet {
    float posX, posY;
    float dirX, dirY;
    float speed;
    SDL_Rect rect;
    bool active;
    bool isEnemy;

    Bullet(float x, float y, float dx, float dy, float spd)
        : posX(x), posY(y), dirX(dx), dirY(dy), speed(spd), active(true), isEnemy(false) {
        rect = { static_cast<int>(x), static_cast<int>(y), 16, 16 };
    }

    void update(float deltaTime) {
        if (!active) return;

        posX += dirX * speed * deltaTime;
        posY += dirY * speed * deltaTime;
        rect.x = static_cast<int>(posX);
        rect.y = static_cast<int>(posY);
    }

    void render(SDL_Renderer* renderer, SDL_Texture* playerBulletTexture, SDL_Texture* enemyBulletTexture) const {
        if (!active) return;

        SDL_Texture* bulletTexture = isEnemy ? enemyBulletTexture : playerBulletTexture;

        for (int i = 0; i < 5; i++) {
            SDL_Rect trailRect = rect;
            trailRect.x -= static_cast<int>(dirX * speed * i * 0.016f);
            trailRect.y -= static_cast<int>(dirY * speed * i * 0.016f);
            int alpha = 255 - (i * 50);
            SDL_SetTextureAlphaMod(bulletTexture, alpha);
            SDL_RenderCopy(renderer, bulletTexture, nullptr, &trailRect);
        }
        SDL_SetTextureAlphaMod(bulletTexture, 255);
    }
};

#endif