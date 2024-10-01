#include <SDL2/SDL.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int ENTITY_SIZE = 16;
const int NUM_NPCS = 4;
const int HITBOX_SIZE = 40;

class Entity {
public:
    int x, y;
    Entity(int startX, int startY) : x(startX), y(startY) {}
    virtual void move() = 0;
};

class Player : public Entity {
public:
    Player(int startX, int startY) : Entity(startX, startY) {}
    void handleInput(SDL_Event &e) {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_w: if (y > 0) y -= ENTITY_SIZE; break;
                case SDLK_s: if (y < SCREEN_HEIGHT - ENTITY_SIZE) y += ENTITY_SIZE; break;
                case SDLK_a: if (x > 0) x -= ENTITY_SIZE; break;
                case SDLK_d: if (x < SCREEN_WIDTH - ENTITY_SIZE) x += ENTITY_SIZE; break;
            }
        }
    }
    void move() override {
        // Player movement is handled by handleInput()
    }
};

class NPC : public Entity {
public:
    NPC(int startX, int startY) : Entity(startX, startY) {}
    void move() override {
        int direction = rand() % 4;
        int newX = x, newY = y;
        switch (direction) {
            case 0: newY -= ENTITY_SIZE; break; // Move up
            case 1: newY += ENTITY_SIZE; break; // Move down
            case 2: newX -= ENTITY_SIZE; break; // Move left
            case 3: newX += ENTITY_SIZE; break; // Move right
        }
        if (canMove(newX, newY)) {
            x = newX;
            y = newY;
        }
    }

    bool canMove(int newX, int newY) {
        if (newX < 0 || newX >= SCREEN_WIDTH || newY < 0 || newY >= SCREEN_HEIGHT) {
            return false;
        }
        for (const auto &npc : npcs) {
            if (npc != this && isCollision(newX, newY, npc->x, npc->y)) {
                return false;
            }
        }
        return true;
    }

    static bool isCollision(int x1, int y1, int x2, int y2) {
        int centerX1 = x1 + ENTITY_SIZE / 2;
        int centerY1 = y1 + ENTITY_SIZE / 2;
        int centerX2 = x2 + ENTITY_SIZE / 2;
        int centerY2 = y2 + ENTITY_SIZE / 2;
        int dx = centerX1 - centerX2;
        int dy = centerY1 - centerY2;
        return std::sqrt(dx * dx + dy * dy) <= HITBOX_SIZE;
    }

    static std::vector<NPC*> npcs;
};

std::vector<NPC*> NPC::npcs;

class Game {
public:
    Game() : player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), gameRunning(true), counter(0) {
        srand(time(0));
        for (int i = 0; i < NUM_NPCS; ++i) {
            npcs.push_back(new NPC(SCREEN_WIDTH / 4 + i * ENTITY_SIZE, SCREEN_HEIGHT / 4 + i * ENTITY_SIZE));
            NPC::npcs.push_back(npcs.back());
        }
    }

    ~Game() {
        for (auto npc : npcs) {
            delete npc;
        }
    }

    void run() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return;
        }

        SDL_Window* window = SDL_CreateWindow("8-bit Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) {
            printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            return;
        }

        SDL_Surface* playerSurface = SDL_CreateRGBSurface(0, ENTITY_SIZE, ENTITY_SIZE, 32, 0, 0, 0, 0);
        SDL_FillRect(playerSurface, NULL, SDL_MapRGB(playerSurface->format, 0xFF, 0x00, 0x00)); // Red player
        SDL_Texture* playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
        SDL_FreeSurface(playerSurface);

        SDL_Surface* npcSurface = SDL_CreateRGBSurface(0, ENTITY_SIZE, ENTITY_SIZE, 32, 0, 0, 0, 0);
        SDL_FillRect(npcSurface, NULL, SDL_MapRGB(npcSurface->format, 0x00, 0xFF, 0x00)); // Green NPC
        SDL_Texture* npcTexture = SDL_CreateTextureFromSurface(renderer, npcSurface);
        SDL_FreeSurface(npcSurface);

        SDL_Event e;

        while (gameRunning) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    gameRunning = false;
                } else {
                    player.handleInput(e);
                }
            }

            updateGame();
            renderGame(renderer, playerTexture, npcTexture);
            SDL_Delay(1000 / 60); // Cap at 60 FPS
        }

        SDL_DestroyTexture(playerTexture);
        SDL_DestroyTexture(npcTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    Player player;
    std::vector<NPC*> npcs;
    bool gameRunning;
    int counter;

    void updateGame() {
        if (counter % 10 == 0) { // Adjust the modulo value to control NPC speed
            for (auto npc : npcs) {
                npc->move();
            }
        }
        counter++;

        // Check for collision between player and NPCs
        for (auto npc : npcs) {
            if (NPC::isCollision(player.x, player.y, npc->x, npc->y)) {
                gameRunning = false;
            }
        }
    }

    void renderGame(SDL_Renderer *renderer, SDL_Texture *playerTexture, SDL_Texture *npcTexture) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        SDL_Rect playerRect = { player.x, player.y, ENTITY_SIZE, ENTITY_SIZE };
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        for (auto npc : npcs) {
            SDL_Rect npcRect = { npc->x, npc->y, ENTITY_SIZE, ENTITY_SIZE };
            SDL_RenderCopy(renderer, npcTexture, NULL, &npcRect);
        }

        SDL_RenderPresent(renderer);
    }
};

int main(int argc, char* args[]) {
    Game game;
    game.run();
    return 0;
}