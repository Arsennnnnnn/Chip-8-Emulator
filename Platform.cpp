#include "platform.h"

#include <iostream>
#include <cstdlib>

namespace {
    // Chip-8's original hex keypad:
    //   1 2 3 C
    //   4 5 6 D
    //   7 8 9 E
    //   A 0 B F
    // Mapped onto the left side of a QWERTY keyboard, the standard scheme
    // most Chip-8 emulators use:
    //   1 2 3 4
    //   Q W E R
    //   A S D F
    //   Z X C V
    int mapScancodeToChip8Key(SDL_Scancode scancode) {
        switch (scancode) {
            case SDL_SCANCODE_1: return 0x1;
            case SDL_SCANCODE_2: return 0x2;
            case SDL_SCANCODE_3: return 0x3;
            case SDL_SCANCODE_4: return 0xC;
            case SDL_SCANCODE_Q: return 0x4;
            case SDL_SCANCODE_W: return 0x5;
            case SDL_SCANCODE_E: return 0x6;
            case SDL_SCANCODE_R: return 0xD;
            case SDL_SCANCODE_A: return 0x7;
            case SDL_SCANCODE_S: return 0x8;
            case SDL_SCANCODE_D: return 0x9;
            case SDL_SCANCODE_F: return 0xE;
            case SDL_SCANCODE_Z: return 0xA;
            case SDL_SCANCODE_X: return 0x0;
            case SDL_SCANCODE_C: return 0xB;
            case SDL_SCANCODE_V: return 0xF;
            default: return -1; // not a Chip-8 key
        }
    }
}

Platform::Platform(const char* title, int textureWidth, int textureHeight, int scaleIn)
    : width(textureWidth), height(textureHeight), scale(scaleIn) {

    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width * scale, height * scale,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(1);
    }
}

Platform::~Platform() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::render(const uint8_t* displayBuffer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (displayBuffer[y * width + x]) {
                SDL_Rect pixel{ x * scale, y * scale, scale, scale };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

bool Platform::processInput(std::array<uint8_t, 16>& keypad) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            const int key = mapScancodeToChip8Key(event.key.keysym.scancode);
            if (key >= 0) {
                keypad[key] = (event.type == SDL_KEYDOWN) ? 1 : 0;
            }
        }
    }
    return true;
}