cmake_minimum_required(VERSION 3.20)
project(vm)

add_executable(secd secd.cpp)
target_include_directories(secd PRIVATE inc)

target_compile_features(secd PRIVATE cxx_std_20)
target_compile_options(secd PRIVATE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(secd PRIVATE -fsanitize=address,undefined)

add_executable(asm assembler.cpp)
target_include_directories(asm PRIVATE inc)

target_compile_features(asm PRIVATE cxx_std_20)
target_compile_options(asm PRIVATE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(asm PRIVATE -fsanitize=address,undefined)

add_subdirectory(src)
