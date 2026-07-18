cmake_minimum_required(VERSION 3.10)
project(Chip-8_Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(chip8 main.cpp chip8.cpp platform.cpp)
target_link_libraries(chip8 SDL2::SDL2)