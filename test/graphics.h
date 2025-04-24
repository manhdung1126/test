#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "init.h"
#include "event.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include "bullet.h"
#include "enemy.h"
#include "game_state.h"

std::string formatTime(int seconds);
std::vector<int> loadScores();

struct GameAssets {
    SDL_Texture* bgTexture = nullptr;
    SDL_Texture* shipTexture = nullptr;
    SDL_Texture* flameTexture = nullptr;
    SDL_Texture* bulletTexture = nullptr;
    SDL_Texture* enemyTexture = nullptr;
    SDL_Texture* healthBarTexture = nullptr;
    SDL_Texture* healthTexture = nullptr;
    TTF_Font* font = nullptr;
    TTF_Font* titleFont = nullptr;
};

inline SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cerr << "Cannot load image " << path << ": " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        std::cerr << "Cannot create texture from " << path << ": " << SDL_GetError() << std::endl;
        return nullptr;
    }
    return texture;
}

inline bool loadAssets(GameAssets& assets, SDL_Renderer* renderer) {
    assets.bgTexture = loadTexture("background.png", renderer);
    if (!assets.bgTexture) return false;

    assets.shipTexture = loadTexture("ships/gray3.png", renderer);
    if (!assets.shipTexture) return false;

    assets.flameTexture = loadTexture("ships/flame.gif", renderer);
    if (!assets.flameTexture) return false;

    assets.bulletTexture = loadTexture("bullet.png", renderer);
    if (!assets.bulletTexture) return false;

    assets.enemyTexture = loadTexture("ships/enemy.png", renderer);
    if (!assets.enemyTexture) return false;

    assets.healthBarTexture = loadTexture("healthBar.png", renderer);
    if (!assets.healthBarTexture) return false;

    assets.healthTexture = loadTexture("health.png", renderer);
    if (!assets.healthTexture) return false;

    assets.font = TTF_OpenFont("arial.ttf", 24);
    if (!assets.font) {
        std::cerr << "Cannot load font arial.ttf: " << TTF_GetError() << std::endl;
        return false;
    }

    assets.titleFont = TTF_OpenFont("arial.ttf", 48);
    if (!assets.titleFont) {
        std::cerr << "Cannot load title font arial.ttf: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

inline void cleanupGraphics(GameAssets& assets) {
    if (assets.bgTexture)      SDL_DestroyTexture(assets.bgTexture);
    if (assets.shipTexture)    SDL_DestroyTexture(assets.shipTexture);
    if (assets.flameTexture)   SDL_DestroyTexture(assets.flameTexture);
    if (assets.bulletTexture)  SDL_DestroyTexture(assets.bulletTexture);
    if (assets.enemyTexture)   SDL_DestroyTexture(assets.enemyTexture);
    if (assets.healthBarTexture) SDL_DestroyTexture(assets.healthBarTexture);
    if (assets.healthTexture)  SDL_DestroyTexture(assets.healthTexture);
    if (assets.font)           TTF_CloseFont(assets.font);
    if (assets.titleFont)      TTF_CloseFont(assets.titleFont);
}

inline void renderScreen(SDL_Renderer* renderer,
    GameAssets& assets,
    Player& player,
    const std::vector<Bullet>& bullets,
    const std::vector<Enemy>& enemies,
    GameState gameState,
    int survivalTime,
    int selectedMenuItem) {
    SDL_RenderClear(renderer);

    if (gameState == PLAYING) {
        SDL_RenderCopy(renderer, assets.bgTexture, nullptr, nullptr);

        if (player.moveUp || player.moveDown || player.moveLeft || player.moveRight) {
            SDL_Rect flameRect = {
                player.rect.x + (player.rect.w - player.rect.w / 2) / 2,
                player.rect.y + player.rect.h / 2 + 80,
                player.rect.w / 2,
                player.rect.h / 4
            };
            SDL_Point center = { flameRect.w / 2, flameRect.h / 2 };
            SDL_RenderCopyEx(renderer, assets.flameTexture, nullptr, &flameRect, player.angle, &center, SDL_FLIP_NONE);
        }

        for (const Bullet& bullet : bullets) {
            bullet.render(renderer, assets.bulletTexture);
        }

        for (const Enemy& enemy : enemies) {
            enemy.render(renderer, assets.enemyTexture);
        }

        SDL_Point shipCenter = { player.rect.w / 2, player.rect.h / 2 };
        SDL_RenderCopyEx(renderer, assets.shipTexture, nullptr, &player.rect, player.angle, &shipCenter, SDL_FLIP_NONE);

        SDL_Rect healthBarRect = { static_cast<int>(WINDOW_WIDTH * 0.65f), static_cast<int>(WINDOW_HEIGHT * 0.1f), static_cast<int>(WINDOW_WIDTH * 0.3f), static_cast<int>(WINDOW_HEIGHT * 0.05f) };
        SDL_RenderCopy(renderer, assets.healthBarTexture, nullptr, &healthBarRect);
        SDL_Rect healthRect = healthBarRect;
        healthRect.w = static_cast<int>(healthBarRect.w * player.health);
        SDL_RenderCopy(renderer, assets.healthTexture, nullptr, &healthRect);

        std::stringstream ss;
        ss << "Time: " << formatTime(survivalTime);
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Surface* textSurface = TTF_RenderText_Solid(assets.font, ss.str().c_str(), white);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = { WINDOW_WIDTH - textSurface->w - 10, 10, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
    else if (gameState == MENU) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color white = { 255,255,255,255 };
        SDL_Color yellow = { 255,255,0,255 };

        auto renderText = [&](TTF_Font* f, const std::string& text, SDL_Color color, int x, int y) {
            SDL_Surface* surf = TTF_RenderText_Solid(f, text.c_str(), color);
            if (!surf) return;
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = { x, y, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, nullptr, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
            };

        renderText(assets.titleFont, "Space Shooter", white, (WINDOW_WIDTH - 300) / 2, 100);
        renderText(assets.font, "Start Game", (selectedMenuItem == 0 ? yellow : white), (WINDOW_WIDTH - 200) / 2, 250);
        renderText(assets.font, "View High Scores", (selectedMenuItem == 1 ? yellow : white), (WINDOW_WIDTH - 300) / 2, 300);
        renderText(assets.font, "Use Arrows to Select, Enter to Confirm", white, (WINDOW_WIDTH - 400) / 2, 400);
    }
    else if (gameState == GAME_OVER) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color white = { 255,255,255,255 };
        auto renderText = [&](TTF_Font* f, const std::string& text, int x, int y) {
            SDL_Surface* surf = TTF_RenderText_Solid(f, text.c_str(), white);
            if (!surf) return;
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = { x, y, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, nullptr, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
            };

        renderText(assets.titleFont, "Game Over", (WINDOW_WIDTH - 300) / 2, 100);
        renderText(assets.font, "Survival Time: " + formatTime(survivalTime), (WINDOW_WIDTH - 400) / 2, 180);

        auto scores = loadScores();
        for (size_t i = 0; i < scores.size(); ++i) {
            renderText(assets.font, "Top " + std::to_string(i + 1) + ": " + formatTime(scores[i]), (WINDOW_WIDTH - 300) / 2, 260 + i * 40);
        }

        if ((SDL_GetTicks() % 1000) < 500) {
            renderText(assets.font, "Press R to Return to Menu", (WINDOW_WIDTH - 400) / 2, 460);
        }
    }
    else if (gameState == VIEW_SCORES) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color white = { 255,255,255,255 };
        auto renderText = [&](TTF_Font* f, const std::string& text, int x, int y) {
            SDL_Surface* surf = TTF_RenderText_Solid(f, text.c_str(), white);
            if (!surf) return;
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = { x, y, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, nullptr, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
            };

        renderText(assets.titleFont, "High Scores", (WINDOW_WIDTH - 300) / 2, 100);
        auto scores = loadScores();
        for (size_t i = 0; i < scores.size(); ++i) {
            renderText(assets.font, "Top " + std::to_string(i + 1) + ": " + formatTime(scores[i]), (WINDOW_WIDTH - 300) / 2, 200 + i * 40);
        }
        renderText(assets.font, "Press Enter to Return", (WINDOW_WIDTH - 400) / 2, 400);
    }

    SDL_RenderPresent(renderer);
}

#endif