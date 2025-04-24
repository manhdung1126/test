#include "init.h"
#include "event.h"
#include "graphics.h"
#include "bullet.h"
#include "enemy.h"
#include "game_state.h"
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>

struct GameData {
    int survivalTime = 0;
    int score = 0;
    int selectedMenuItem = 0;
    Uint32 lastSpawnTime = 0;
    Uint32 gameStartTime = 0; // Thêm để theo dõi thời điểm bắt đầu game
    const Uint32 SPAWN_INTERVAL = 3000;
};

bool intersectBullet(float bulletX, float bulletY, float shipX, float shipY, float shipSize) {
    float dx = bulletX - shipX;
    float dy = bulletY - shipY;
    return std::sqrt(dx * dx + dy * dy) <= shipSize;
}

std::string formatTime(int seconds) {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setw(2) << secs;
    return ss.str();
}

void saveScore(int score) {
    std::ofstream file("scores.txt", std::ios::app);
    if (file.is_open()) {
        file << score << "\n";
        file.close();
    }
}

std::vector<int> loadScores() {
    std::vector<int> scores;
    std::ifstream file("scores.txt");
    int score;
    while (file >> score) {
        scores.push_back(score);
    }
    file.close();
    std::sort(scores.begin(), scores.end(), std::greater<int>());
    if (scores.size() > 5) scores.resize(5);
    return scores;
}

void spawnEnemy(std::vector<Enemy>& enemies, std::mt19937& gen,
    std::uniform_real_distribution<float>& posDistX,
    std::uniform_real_distribution<float>& posDistY,
    std::uniform_real_distribution<float>& speedDist,
    std::uniform_real_distribution<float>& radiusDist,
    std::uniform_real_distribution<float>& orbitSpeedDist) {
    float x = posDistX(gen);
    float y = posDistY(gen);
    float speed = speedDist(gen);
    float radius = radiusDist(gen);
    float orbitSpeed = orbitSpeedDist(gen);
    enemies.emplace_back(x, y, speed, radius, orbitSpeed);
}

void resetGame(Player& player, std::vector<Bullet>& bullets, std::vector<Enemy>& enemies,
    GameData& gameData, std::mt19937& gen,
    std::uniform_real_distribution<float>& posDistX,
    std::uniform_real_distribution<float>& posDistY,
    std::uniform_real_distribution<float>& speedDist,
    std::uniform_real_distribution<float>& radiusDist,
    std::uniform_real_distribution<float>& orbitSpeedDist) {
    player = Player();
    bullets.clear();
    enemies.clear();
    enemies.emplace_back(posDistX(gen), posDistY(gen), speedDist(gen), radiusDist(gen), orbitSpeedDist(gen));
    gameData.lastSpawnTime = 0;
    gameData.survivalTime = 0;
    gameData.score = 0;
    gameData.gameStartTime = SDL_GetTicks(); // Reset thời điểm bắt đầu game
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initGame(&window, &renderer)) return -1;

    GameAssets assets;
    if (!loadAssets(assets, renderer)) {
        cleanUp(window, renderer);
        return -1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> speedDist(150.0f, 300.0f);
    std::uniform_real_distribution<float> radiusDist(200.0f, 400.0f);
    std::uniform_real_distribution<float> orbitSpeedDist(0.01f, 0.05f);
    std::uniform_real_distribution<float> posDistX(0.0f, WINDOW_WIDTH);
    std::uniform_real_distribution<float> posDistY(0.0f, WINDOW_HEIGHT);

    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    GameState gameState = MENU;
    GameData gameData;
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    const int FPS = 60;
    const float FRAME_TIME = 1000.0f / FPS;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (gameState == MENU) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        gameData.selectedMenuItem = (gameData.selectedMenuItem == 0) ? 1 : 0;
                    }
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        gameData.selectedMenuItem = (gameData.selectedMenuItem == 0) ? 1 : 0;
                    }
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        if (gameData.selectedMenuItem == 0) {
                            resetGame(player, bullets, enemies, gameData, gen, posDistX, posDistY, speedDist, radiusDist, orbitSpeedDist);
                            gameState = PLAYING;
                        }
                        else {
                            gameState = VIEW_SCORES;
                        }
                    }
                }
                else if (gameState == GAME_OVER) {
                    if (event.key.keysym.sym == SDLK_r) {
                        gameState = MENU;
                        gameData.selectedMenuItem = 0;
                        resetGame(player, bullets, enemies, gameData, gen, posDistX, posDistY, speedDist, radiusDist, orbitSpeedDist);
                    }
                }
                else if (gameState == VIEW_SCORES) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        gameState = MENU;
                    }
                }
            }
            if (gameState == PLAYING) {
                handleEvent(event, running, player, bullets);
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = std::min((currentTime - lastTime) / 1000.0f, 0.033f);
        lastTime = currentTime;

        if (gameState == PLAYING) {
            gameData.survivalTime = (currentTime - gameData.gameStartTime) / 1000; // Tính thời gian từ gameStartTime

            updatePlayer(player, deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT, bullets);

            if (currentTime - gameData.lastSpawnTime >= gameData.SPAWN_INTERVAL) {
                spawnEnemy(enemies, gen, posDistX, posDistY, speedDist, radiusDist, orbitSpeedDist);
                gameData.lastSpawnTime = currentTime;
            }

            for (Enemy& enemy : enemies) {
                enemy.update(deltaTime, player.posX, player.posY, bullets);
            }

            for (Bullet& bullet : bullets) {
                bullet.update(deltaTime);
            }

            for (size_t i = 0; i < bullets.size();) {
                bool bulletRemoved = false;
                float dist = std::sqrt(std::pow(bullets[i].posX - (player.posX + player.rect.w / 2), 2) +
                    std::pow(bullets[i].posY - (player.posY + player.rect.h / 2), 2));
                if (dist > 2000) {
                    bullets.erase(bullets.begin() + i);
                    continue;
                }

                if (!bullets[i].isEnemy) {
                    for (size_t e = 0; e < enemies.size(); e++) {
                        if (intersectBullet(bullets[i].posX, bullets[i].posY, enemies[e].posX, enemies[e].posY, ENEMY_SHIP_SIZE)) {
                            enemies[e].life -= 0.1f;
                            if (enemies[e].life <= 0) {
                                enemies.erase(enemies.begin() + e);
                                gameData.score += 100;
                            }
                            bullets.erase(bullets.begin() + i);
                            bulletRemoved = true;
                            break;
                        }
                    }
                }
                else {
                    if (intersectBullet(bullets[i].posX, bullets[i].posY, player.posX + player.rect.w / 2, player.posY + player.rect.h / 2, 64.0f)) {
                        player.health -= 0.1f;
                        bullets.erase(bullets.begin() + i);
                        bulletRemoved = true;
                    }
                }
                if (!bulletRemoved) ++i;
            }

            player.health += deltaTime * 0.05f;
            if (player.health > 1.0f) player.health = 1.0f;
            if (player.health < 0.0f) player.health = 0.0f;

            if (player.health <= 0) {
                saveScore(gameData.score);
                gameState = GAME_OVER;
            }

            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                [](const Enemy& e) {
                    return e.posX < -100 || e.posX > 1380 ||
                        e.posY < -100 || e.posY > 820;
                }),
                enemies.end());

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            updateRotation(player, mouseX, mouseY);
        }

        renderScreen(renderer, assets, player, bullets, enemies, gameState, gameData.survivalTime, gameData.selectedMenuItem, gameData.score);

        Uint32 frameEnd = SDL_GetTicks();
        float frameDuration = frameEnd - currentTime;
        if (frameDuration < FRAME_TIME) {
            SDL_Delay(static_cast<Uint32>(FRAME_TIME - frameDuration));
        }
    }

    cleanupGraphics(assets);
    cleanUp(window, renderer);
    return 0;
}