cmake_minimum_required(VERSION 3.16)
project(settlers C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Downloads and builds raylib automatically on first configure
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(raylib)

add_executable(settlers main.cpp board.cpp game.cpp player.cpp render.cpp)
target_link_libraries(settlers PRIVATE raylib)
