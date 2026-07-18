# Chip-8 Emulator

A Chip-8 virtual machine interpreter written in modern C++, with SDL2-based graphics and input.

## Overview

Chip-8 is an interpreted virtual machine from the mid-1970s, originally used to run simple games (Pong, Tetris, Space Invaders) on 8-bit hobbyist computers. Writing a Chip-8 interpreter is a classic entry point into low-level systems programming: it requires implementing a fetch-decode-execute cycle, managing memory and registers by hand, and reasoning about instruction semantics at the bit level, without the scale of a real CPU architecture getting in the way.

This project implements the full standard Chip-8 instruction set (35 opcodes), a 64x32 monochrome display, 16-key input, and the two hardware timers, and runs original Chip-8 ROMs.

## Features

- All 35 standard Chip-8 opcodes implemented
- Fetch-decode-execute CPU loop, with instruction execution decoupled from the fixed 60Hz timer clock
- Correct carry/borrow/collision flag (`VF`) semantics for arithmetic and drawing instructions
- Sprite rendering via XOR, with edge clipping and collision detection
- Blocking key-wait (`FX0A`) implemented without stalling the render loop
- Clean separation between emulation logic and platform/rendering code

## Architecture

The project is split into two independent layers:

| Component | Responsibility |
|---|---|
| `Chip8` (`chip8.h`/`.cpp`) | Pure interpreter: memory, registers, timers, and all opcode logic. No knowledge of SDL2 or any rendering backend. |
| `Platform` (`platform.h`/`.cpp`) | Owns the SDL2 window, renderer, and keyboard mapping. Exposes a small interface (`render()`, `processInput()`) so the interpreter never touches SDL directly. |
| `main.cpp` | Orchestration only — loads the ROM, wires the two together, runs the main loop. |

Within `Chip8`, the instruction dispatcher (`execute()`) stays flat: trivial single-line instructions are handled inline, while the four opcode families with real branching logic (`8XY_` register math, `DXYN` sprite drawing, `EX__` key checks, `FX__` timers/memory) each have their own method.

## Project Structure

```
.
├── main.cpp         # Entry point and main loop
├── chip8.h          # Chip8 class declaration
├── chip8.cpp         # CPU state, opcode dispatch, and instruction implementations
├── platform.h        # SDL2 wrapper class declaration
├── platform.cpp       # Window/renderer setup, drawing, input mapping
└── CMakeLists.txt     # Build configuration
```

## Requirements

- C++17 compiler
- CMake 3.10+
- SDL2

**Windows:** [vcpkg](https://github.com/microsoft/vcpkg) is the recommended way to install SDL2.
**Linux:** install via your package manager, e.g. `sudo apt install libsdl2-dev`.

## Building

### Windows (vcpkg)

```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe install sdl2:x64-windows
```

Then configure CMake with the vcpkg toolchain file:

```powershell
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Linux

```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
cmake -B build
cmake --build build
```

## Usage

```bash
./chip8 path/to/rom.ch8
```

### Controls

Chip-8's original hex keypad is mapped onto the left side of a QWERTY keyboard:

| Chip-8 keypad |   |   |   |     | Keyboard |   |   |   |
|:---:|:---:|:---:|:---:|---|:---:|:---:|:---:|:---:|
| 1 | 2 | 3 | C |     | 1 | 2 | 3 | 4 |
| 4 | 5 | 6 | D |     | Q | W | E | R |
| 7 | 8 | 9 | E |     | A | S | D | F |
| A | 0 | B | F |     | Z | X | C | V |

## Testing

Correctness was verified in two stages:

1. **Targeted unit tests** — hand-assembled minimal ROMs exercising carry/borrow flags, BCD digit conversion, register block store/load, and key-skip logic, checked against manually computed expected results.
2. **[Timendus' chip8-test-suite](https://github.com/Timendus/chip8-test-suite)** — a community-maintained reference ROM suite covering opcode correctness, flag behavior, and keypad handling.

## Known Limitations

- **Sound:** the sound timer decrements correctly, but no audio output is wired up yet — `FX18` and timer behavior are accurate, there's just no beep.
- **Quirks:** implements the modern/common interpretation of a few historically ambiguous instructions (e.g. `8XY6`/`8XYE` shift without referencing `Vy`; `FX55`/`FX65` without incrementing `I` afterward). A small number of original COSMAC VIP-era ROMs that depend on the alternate behavior may not run correctly as a result.

## Roadmap

- [ ] SDL2 audio callback for the sound timer
- [ ] Configurable clock speed
- [ ] Save states

## References

- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Timendus' Chip-8 test suite](https://github.com/Timendus/chip8-test-suite)
- [chip8Archive](https://github.com/JohnEarnest/chip8Archive) — public domain Chip-8 ROMs

## Author

**Arsen Avetisyan**

https://github.com/Arsennnnnnn / www.linkedin.com/in/arsenavetisyann
