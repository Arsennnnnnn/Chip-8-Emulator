cmake_minimum_required(VERSION 3.31)
project(Chip_8_Emulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(Chip_8_Emulator main.cpp
        Chip8.cpp
        Chip8.h)
