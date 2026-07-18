#include "chip8.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

namespace {
    constexpr unsigned int START_ADDRESS = 0x200;
    constexpr unsigned int FONTSET_START_ADDRESS = 0x50;
    constexpr unsigned int FONTSET_SIZE = 80;

    uint8_t fontset[FONTSET_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
}

Chip8::Chip8() {
    init();
}

void Chip8::init() {
    pc = START_ADDRESS;
    opcode = 0;
    I = 0;
    sp = 0;

    memory.fill(0);
    V.fill(0);
    stack.fill(0);
    display.fill(0);
    keypad.fill(0);

    loadFontset();

    delayTimer = 0;
    soundTimer = 0;
    drawFlag = false;

    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void Chip8::loadFontset() {
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

bool Chip8::loadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open ROM: " << path << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size <= 0 || static_cast<size_t>(size) > memory.size() - START_ADDRESS) {
        std::cerr << "ROM is empty or too large to fit in memory" << std::endl;
        return false;
    }

    std::vector<char> buffer(static_cast<size_t>(size));
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read ROM" << std::endl;
        return false;
    }

    for (std::streamsize i = 0; i < size; ++i) {
        memory[START_ADDRESS + i] = static_cast<uint8_t>(buffer[i]);
    }

    return true;
}

void Chip8::emulateCycle() {
    // Fetch: two consecutive bytes form one 16-bit opcode
    opcode = static_cast<uint16_t>((memory[pc] << 8) | memory[pc + 1]);
    pc += 2;
    // Decode + execute
    execute();
}

void Chip8::execute() {
    const uint8_t x   = (opcode & 0x0F00) >> 8;
    const uint8_t y   = (opcode & 0x00F0) >> 4;
    const uint8_t n   =  opcode & 0x000F;
    const uint8_t nn  =  opcode & 0x00FF;
    const uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {

        case 0x0000:
            switch (opcode) {
                case 0x00E0: // CLS - clear the display
                    display.fill(0);
                    drawFlag = true;
                    break;
                case 0x00EE: // RET - return from subroutine
                    --sp;
                    pc = stack[sp];
                    break;
                default:
                    break;
            }
            break;

        case 0x1000:
            pc = nnn;
            break;
        case 0x2000:
            stack[sp] = pc;
            ++sp;
            pc = nnn;
            break;
        case 0x3000:
            if (V[x] == nn) pc += 2;
            break;
        case 0x4000:
            if (V[x] != nn) pc += 2;
            break;
        case 0x5000:
            if (V[x] == V[y]) pc += 2;
            break;
        case 0x6000:
            V[x] = nn;
            break;
        case 0x7000:
            V[x] = static_cast<uint8_t>(V[x] + nn);
            break;
        case 0x8000:
            op8xy(x, y, n);
            break;
        case 0x9000:
            if (V[x] != V[y]) pc += 2;
            break;
        case 0xA000:
            I = nnn;
            break;
        case 0xB000:
            pc = nnn + V[0];
            break;
        case 0xC000:
            V[x] = static_cast<uint8_t>((std::rand() % 256) & nn);
            break;
        case 0xD000:
            opDxyn(x, y, n);
            break;
        case 0xE000:
            opExnn(x, nn);
            break;
        case 0xF000:
            opFxnn(x, nn);
            break;
        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            break;
    }
}

void Chip8::op8xy(uint8_t x, uint8_t y, uint8_t n) {
    switch (n) {
        case 0x0:
            V[x] = V[y];
            break;

        case 0x1:
            V[x] |= V[y];
            break;

        case 0x2:
            V[x] &= V[y];
            break;

        case 0x3:
            V[x] ^= V[y];
            break;

        case 0x4: {
            const uint16_t sum = static_cast<uint16_t>(V[x]) + V[y];
            V[0xF] = (sum > 0xFF) ? 1 : 0;
            V[x] = static_cast<uint8_t>(sum & 0xFF);
            break;
        }

        case 0x5: {
            const uint8_t flag = (V[x] >= V[y]) ? 1 : 0;
            const uint8_t result = static_cast<uint8_t>(V[x] - V[y]);
            V[0xF] = flag;
            V[x] = result;
            break;
        }

        case 0x6: {
            const uint8_t flag = V[x] & 0x1;
            const uint8_t result = static_cast<uint8_t>(V[x] >> 1);
            V[0xF] = flag;
            V[x] = result;
            break;
        }

        case 0x7: {
            const uint8_t flag = (V[y] >= V[x]) ? 1 : 0;
            const uint8_t result = static_cast<uint8_t>(V[y] - V[x]);
            V[0xF] = flag;
            V[x] = result;
            break;
        }

        case 0xE: {
            const uint8_t flag = (V[x] & 0x80) >> 7;
            const uint8_t result = static_cast<uint8_t>(V[x] << 1);
            V[0xF] = flag;
            V[x] = result;
            break;
        }

        default:
            std::cerr << "Unknown 0x8000 opcode, last nibble: 0x" << std::hex << (int)n << std::dec << std::endl;
            break;
    }
}

void Chip8::opDxyn(uint8_t x, uint8_t y, uint8_t n) {
    const uint8_t xPos = V[x] % VIDEO_WIDTH;
    const uint8_t yPos = V[y] % VIDEO_HEIGHT;
    V[0xF] = 0;

    for (uint8_t row = 0; row < n; ++row) {
        if (yPos + row >= VIDEO_HEIGHT) break;
        const uint8_t spriteByte = memory[I + row];

        for (uint8_t col = 0; col < 8; ++col) {
            if (xPos + col >= VIDEO_WIDTH) break;

            const uint8_t spritePixel = (spriteByte >> (7 - col)) & 0x1;
            if (!spritePixel) continue;

            const int idx = (yPos + row) * VIDEO_WIDTH + (xPos + col);
            if (display[idx] == 1) {
                V[0xF] = 1;
            }
            display[idx] ^= 1;
        }
    }

    drawFlag = true;
}

void Chip8::opExnn(uint8_t x, uint8_t nn) {
    switch (nn) {
        case 0x9E:
            if (keypad[V[x] & 0xF]) pc += 2;
            break;

        case 0xA1:
            if (!keypad[V[x] & 0xF]) pc += 2;
            break;

        default:
            std::cerr << "Unknown 0xE000 opcode, low byte: 0x" << std::hex << (int)nn << std::dec << std::endl;
            break;
    }
}

void Chip8::opFxnn(uint8_t x, uint8_t nn) {
    switch (nn) {
        case 0x07:
            V[x] = delayTimer;
            break;
        case 0x0A: {
            bool keyPressed = false;
            for (uint8_t k = 0; k < 16; ++k) {
                if (keypad[k]) {
                    V[x] = k;
                    keyPressed = true;
                    break;
                }
            }
            if (!keyPressed) {
                pc -= 2;
            }
            break;
        }
        case 0x15:
            delayTimer = V[x];
            break;
        case 0x18:
            soundTimer = V[x];
            break;
        case 0x1E:
            I = static_cast<uint16_t>(I + V[x]);
            break;
        case 0x29:
            I = static_cast<uint16_t>(FONTSET_START_ADDRESS + (V[x] & 0xF) * 5);
            break;
        case 0x33: {
            uint8_t value = V[x];
            memory[I + 2] = value % 10; value /= 10;
            memory[I + 1] = value % 10; value /= 10;
            memory[I]     = value % 10;
            break;
        }
        case 0x55:
            for (uint16_t i = 0; i <= x; ++i) {
                memory[I + i] = V[i];
            }
            break;
        case 0x65:
            for (uint16_t i = 0; i <= x; ++i) {
                V[i] = memory[I + i];
            }
            break;
        default:
            std::cerr << "Unknown 0xF000 opcode, low byte: 0x" << std::hex << (int)nn << std::dec << std::endl;
            break;
    }
}

void Chip8::updateTimers() {
    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}