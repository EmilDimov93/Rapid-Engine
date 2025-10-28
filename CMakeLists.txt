cmake_minimum_required(VERSION 3.10)
project(RapidEngine C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(Engine/raylib)

add_executable(RapidEngine
    Engine/unity.c
)

target_link_libraries(RapidEngine raylib)

if(WIN32)
    target_link_libraries(RapidEngine opengl32 gdi32 winmm)
endif()