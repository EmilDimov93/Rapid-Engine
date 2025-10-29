cmake_minimum_required(VERSION 3.10)
project(RapidEngine C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(Engine/raylib)

add_executable(RapidEngine
    Engine/Engine.c
    Engine/CGEditor.c
    Engine/Interpreter.c
    Engine/HitboxEditor.c
    Engine/ProjectManager.c
    Engine/TextEditor.c
    Engine/Nodes.c
    Engine/InfoByType.c
    Engine/definitions.c
    Engine/resources/fonts.c
    Engine/resources/sound.c
    Engine/resources/textures.c
)

target_link_libraries(RapidEngine raylib)

if(WIN32)
    target_link_libraries(RapidEngine opengl32 gdi32 winmm)
endif()

set(STACK_SIZE 8388608)

if(MSVC)
    target_link_options(RapidEngine PRIVATE /STACK:${STACK_SIZE})
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    if(WIN32)
        target_link_options(RapidEngine PRIVATE "-Wl,--stack,${STACK_SIZE}")
        target_link_options(RapidEngine PRIVATE "-mwindows")
    endif()
endif()