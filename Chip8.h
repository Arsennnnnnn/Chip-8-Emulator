#pragma once

#include <cstdint>
#include <array>
#include <string>

class Chip8 {
public:
    static constexpr int VIDEO_WIDTH = 64;
    static constexpr int VIDEO_HEIGHT = 32;

    Chip8();

    void init();
    bool loadROM(const std::string& path);
    void emulateCycle();
    void updateTimers();

    std::array<uint8_t, VIDEO_WIDTH * VIDEO_HEIGHT> display{};
    bool drawFlag = false;

    std::array<uint8_t, 16> keypad{};

private:

    std::array<uint8_t, 4096> memory{}; // Memory: 4KB total

    std::array<uint8_t, 16> V{}; // General purpose registers V0-VF

    uint16_t I = 0; // Index register

    uint16_t pc = 0x200; // Program counter: address of the next instruction to fetch

    std::array<uint16_t, 16> stack{};
    uint8_t sp = 0;
    uint8_t delayTimer = 0;
    uint8_t soundTimer = 0;
    uint16_t opcode = 0;

    void loadFontset();

    void execute();

    void op8xy(uint8_t x, uint8_t y, uint8_t n); // register math: OR/AND/XOR/ADD/SUB/SHR/SUBN/SHL
    void opDxyn(uint8_t x, uint8_t y, uint8_t n); // DRW - sprite drawing
    void opExnn(uint8_t x, uint8_t nn);           // SKP/SKNP - key checks
    void opFxnn(uint8_t x, uint8_t nn);           // timers, BCD, font lookup, memory block ops
};