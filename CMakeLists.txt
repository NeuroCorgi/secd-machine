cmake_minimum_required(VERSION 3.20)
project(vm)

add_executable(secd secd.cpp)
add_subdirectory(src)
target_include_directories(secd PRIVATE inc)
target_compile_features(secd PRIVATE cxx_std_20)
target_compile_options(secd PRIVATE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(secd PRIVATE -fsanitize=address,undefined)

add_executable(f f.cpp)
target_include_directories(f PRIVATE inc)

target_compile_features(f PRIVATE cxx_std_20)
target_compile_options(f PRIVATE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(f PRIVATE -fsanitize=address,undefined)