cmake_minimum_required(VERSION 3.10)
project(RapidEngine C)

if(MSVC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Unsupported compiler. Use GCC (MinGW on Windows).")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 99)

add_subdirectory(Engine/raylib)

add_executable(RapidEngine
    Engine/unity.c
)

target_link_libraries(RapidEngine raylib)

if(WIN32)
    target_link_libraries(RapidEngine opengl32 gdi32 winmm)
endif()