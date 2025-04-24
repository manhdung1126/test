#ifndef ENEMY_H
#define ENEMY_H

#include <SDL.h>
#include "bullet.h"
#include "graphics.h"
#include <cmath>

constexpr float ENEMY_SHIP_SIZE = 64.0f;

struct Enemy {
    float posX, posY;
    float dirX, dirY;
    float speed;
    float turnSpeed = 3.0f;
    float fireTimer = 0.0f;
    float fireTimeReset = 0.5f;
    float fireRange = 0.9f;  // Đã sửa từ 1.5f thành 0.9f
    float bulletSpeed = 500.0f;
    float life = 1.0f;
    SDL_Rect rect;
    float orbitRadius;
    float orbitSpeed;
    float angle = 0.0f;

    Enemy(float x, float y, float spd, float radius, float orbitSpd)
        : posX(x), posY(y), dirX(1.0f), dirY(0.0f), speed(spd), orbitRadius(radius), orbitSpeed(orbitSpd) {
        rect = { static_cast<int>(posX), static_cast<int>(posY), 64, 64 };
    }

    bool update(float deltaTime, float playerX, float playerY, std::vector<Bullet>& bullets) {
        float dx = playerX - posX;
        float dy = playerY - posY;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance == 0) { dx = 1; dy = 0; }
        else { dx /= distance; dy /= distance; }

        float dot = dx * dirX + dy * dirY;
        bool shoot = (dot >= fireRange);
        if (shoot && fireTimer <= 0.0f) {
            Bullet b(posX + rect.w / 2, posY + rect.h / 2, dirX, dirY, bulletSpeed);
            b.active = true;
            b.isEnemy = true;
            bullets.push_back(b);
            fireTimer = fireTimeReset;
            return true;
        }

        fireTimer -= deltaTime;
        if (fireTimer < 0) fireTimer = 0.0f;

        float newDirX = deltaTime * turnSpeed * dx + dirX;
        float newDirY = deltaTime * turnSpeed * dy + dirY;
        float length = std::sqrt(newDirX * newDirX + newDirY * newDirY);
        if (length != 0) {
            dirX = newDirX / length;
            dirY = newDirY / length;
        }

        angle += orbitSpeed;
        float orbitX = playerX + orbitRadius * std::cos(angle);
        float orbitY = playerY + orbitRadius * std::sin(angle);
        if (distance > 50.0f) {
            posX += dx * speed * deltaTime;
            posY += dy * speed * deltaTime;
        }
        posX += (orbitX - posX) * 0.05f;
        posY += (orbitY - posY) * 0.05f;

        rect.x = static_cast<int>(posX);
        rect.y = static_cast<int>(posY);

        return false;
    }

    void render(SDL_Renderer* renderer, SDL_Texture* enemyTexture) const {
        float angle = std::atan2(dirY, dirX) * 180.0f / M_PI + 90.0f;
        SDL_Point center = { rect.w / 2, rect.h / 2 };
        SDL_RenderCopyEx(renderer, enemyTexture, nullptr, &rect, angle, &center, SDL_FLIP_NONE);
    }
};

#endif