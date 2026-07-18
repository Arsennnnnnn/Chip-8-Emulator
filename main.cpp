#include <iostream>
#include <chrono>
#include <thread>

#include "chip8.h"
#include "platform.h"

namespace {
    constexpr int VIDEO_WIDTH = 64;
    constexpr int VIDEO_HEIGHT = 32;
    constexpr int SCALE = 10;              // window = 640x320
    constexpr int CYCLES_PER_FRAME = 10;   // instructions executed per render frame
    constexpr int TIMER_HZ = 60;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-rom.ch8>" << std::endl;
        return 1;
    }

    Chip8 chip8;
    if (!chip8.loadROM(argv[1])) {
        return 1;
    }

    Platform platform("Chip-8 Emulator", VIDEO_WIDTH, VIDEO_HEIGHT, SCALE);

    bool running = true;
    auto lastTimerUpdate = std::chrono::steady_clock::now();

    while (running) {
        running = platform.processInput(chip8.keypad);

        for (int i = 0; i < CYCLES_PER_FRAME; ++i) {
            chip8.emulateCycle();
        }

        auto now = std::chrono::steady_clock::now();
        if (now - lastTimerUpdate >= std::chrono::milliseconds(1000 / TIMER_HZ)) {
            chip8.updateTimers();
            lastTimerUpdate = now;
        }

        // Only touch the GPU when the display buffer actually changed
        if (chip8.drawFlag) {
            platform.render(chip8.display.data());
            chip8.drawFlag = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}