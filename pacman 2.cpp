#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int ENTITY_SIZE = 32;
const double PLAYER_SPEED = 0.1;
const double NPC_SPEED = 0.1;

class Entity {
public:
    double x, y;
    virtual void move() = 0;
};

class Player : public Entity {
public:
    void move() override {
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        if (currentKeyStates[SDL_SCANCODE_UP]) {
            y -= PLAYER_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN]) {
            y += PLAYER_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT]) {
            x -= PLAYER_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
            x += PLAYER_SPEED;
        }

        // Ensure the player stays within the screen bounds
        if (x < 0) x = 0;
        if (x > SCREEN_WIDTH - ENTITY_SIZE) x = SCREEN_WIDTH - ENTITY_SIZE;
        if (y < 0) y = 0;
        if (y > SCREEN_HEIGHT - ENTITY_SIZE) y = SCREEN_HEIGHT - ENTITY_SIZE;
    }
};

class NPC : public Entity {
public:
    NPC() : direction(rand() % 4), steps(0) {}

    void move() override {
        // More natural movement logic for NPCs
        if (steps <= 0) {
            direction = rand() % 4;
            steps = rand() % 1000 + 100; // Change direction after 10 to 60 steps
        }

        switch (direction) {
            case 0: y -= NPC_SPEED; break; // Move up
            case 1: y += NPC_SPEED; break; // Move down
            case 2: x -= NPC_SPEED; break; // Move left
            case 3: x += NPC_SPEED; break; // Move right
        }
        steps--;

        // Ensure the NPC stays within the screen bounds
        if (x < 0) x = 0;
        if (x > SCREEN_WIDTH - ENTITY_SIZE) x = SCREEN_WIDTH - ENTITY_SIZE;
        if (y < 0) y = 0;
        if (y > SCREEN_HEIGHT - ENTITY_SIZE) y = SCREEN_HEIGHT - ENTITY_SIZE;
    }

    static bool isCollision(int x1, int y1, int x2, int y2) {
        return x1 < x2 + ENTITY_SIZE &&
               x1 + ENTITY_SIZE > x2 &&
               y1 < y2 + ENTITY_SIZE &&
               y1 + ENTITY_SIZE > y2;
    }

private:
    int direction;
    int steps;
};

class Game {
public:
    bool gameRunning;
    Player player;
    std::vector<NPC*> npcs;
    int counter;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* playerTexture;
    SDL_Texture* npcTexture;

    Game() : gameRunning(true), counter(0), window(nullptr), renderer(nullptr), playerTexture(nullptr), npcTexture(nullptr) {
        // Initialize player and NPCs
        player.x = SCREEN_WIDTH / 2;
        player.y = SCREEN_HEIGHT / 2;
        // Add NPCs to the vector
        for (int i = 0; i < 5; ++i) {
            NPC* npc = new NPC();
            npc->x = rand() % SCREEN_WIDTH;
            npc->y = rand() % SCREEN_HEIGHT;
            npcs.push_back(npc);
        }
    }

    ~Game() {
        for (auto npc : npcs) {
            delete npc;
        }
    }

    void run() {
        if (!init()) {
            std::cerr << "Failed to initialize!" << std::endl;
            return;
        }

        playerTexture = loadTexture("/Users/lukemacvicar/Desktop/pac.png");
        npcTexture = loadTexture("/Users/lukemacvicar/Desktop/ghost.png");

        if (playerTexture == nullptr || npcTexture == nullptr) {
            std::cerr << "Failed to load textures!" << std::endl;
            return;
        }

        SDL_Event e;
        while (gameRunning) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    gameRunning = false;
                }
            }

            player.move();
            for (auto npc : npcs) {
                npc->move();
            }
            counter++;

            // Check for collision between player and NPCs
            for (auto npc : npcs) {
                if (NPC::isCollision(player.x, player.y, npc->x, npc->y)) {
                    gameRunning = false;
                }
            }

            renderGame();
        }

        close();
    }

private:
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            return false;
        }

        srand(static_cast<unsigned int>(time(0))); // Seed for random number generation

        return true;
    }

    SDL_Texture* loadTexture(std::string path) {
        SDL_Texture* newTexture = nullptr;
        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == nullptr) {
            std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        } else {
            newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
            if (newTexture == nullptr) {
                std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
            }
            SDL_FreeSurface(loadedSurface);
        }
        return newTexture;
    }

    void renderGame() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        SDL_Rect playerRect = { static_cast<int>(player.x), static_cast<int>(player.y), ENTITY_SIZE, ENTITY_SIZE };
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        for (auto npc : npcs) {
            SDL_Rect npcRect = { static_cast<int>(npc->x), static_cast<int>(npc->y), ENTITY_SIZE, ENTITY_SIZE };
            SDL_RenderCopy(renderer, npcTexture, NULL, &npcRect);
        }

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyTexture(playerTexture);
        SDL_DestroyTexture(npcTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }
};

int main(int argc, char* args[]) {
    Game game;
    game.run();
    return 0;
}