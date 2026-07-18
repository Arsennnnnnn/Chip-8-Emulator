#pragma once

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <array>
#include <cstdint>

// Wraps every SDL-specific concern (window, renderer, drawing, key mapping)
// so the rest of the program never touches the SDL API directly. main.cpp
// just calls processInput()/render() and stays focused on orchestration.

class Platform {
public:
    Platform(const char* title, int textureWidth, int textureHeight, int scale);
    ~Platform();

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    void render(const uint8_t* displayBuffer);
    bool processInput(std::array<uint8_t, 16>& keypad);

private:

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int width;
    int height;
    int scale;
};