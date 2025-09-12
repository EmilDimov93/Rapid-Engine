#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "raylib.h"
#include "local_config.h"
#include "resources/resources.h"

#if DEVELOPER_MODE
static bool developerMode = true;
#else
static bool developerMode = false;
#endif

#define MAX_FILE_NAME 256
#define MAX_FILE_PATH 2048

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#include <direct.h>
#define MAKE_DIR(path) _mkdir(path)
#define GetCWD _getcwd

void* __stdcall ShellExecuteA(void* hwnd, const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd);

static inline void OpenFile(const char* filePath) {
    ShellExecuteA(NULL, "open", filePath, NULL, NULL, 1);
}

#elif __APPLE__
#define PATH_SEPARATOR '/'
#include <sys/stat.h>
#include <sys/types.h>
#define MAKE_DIR(path) mkdir(path, 0755)
#define GetCWD getcwd

static inline void OpenFile(const char* filePath) {
    char command[1024];
    snprintf(command, sizeof(command), "open \"%s\"", filePath);
    system(command);
}

#elif __unix__
#define PATH_SEPARATOR '/'
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MAKE_DIR(path) mkdir(path, 0755)
#define GetCWD getcwd

static inline void OpenFile(const char* filePath) {
    char command[1024];
    snprintf(command, sizeof(command), "xdg-open \"%s\"", filePath);
    system(command);
}

#else
#error "Rapid Engine supports only Windows, macOS, and Unix-like systems"
#endif

#define MAX_VARIABLE_NAME_SIZE 128
#define MAX_LITERAL_NODE_FIELD_SIZE 512

#define MAX_POLYGON_VERTICES 64

typedef struct {
    Vector2 vertices[MAX_POLYGON_VERTICES];
    int count;
    bool isClosed;
} Polygon;

#define MAX_LOG_MESSAGE_SIZE 256
#define MAX_LOG_MESSAGES 32
typedef enum
{
    LOG_LEVEL_NORMAL,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_SUCCESS,
    LOG_LEVEL_DEBUG
}LogLevel;

extern bool STRING_ALLOCATION_FAILURE;

char* strmac(char* buf, size_t max_size, const char* format, ...) {
    static const char empty[] = "";

    if (!format || max_size == 0){
        STRING_ALLOCATION_FAILURE = true;
        return (buf) ? buf : strdup(empty);
    }

    char* temp = buf;
    bool needs_free = false;

    if (!buf) {
        temp = malloc(max_size);
        if (!temp){
            STRING_ALLOCATION_FAILURE = true;
            return strdup(empty);
        }
        needs_free = true;
    }

    va_list args;
    va_start(args, format);
    int written = vsnprintf(temp, max_size, format, args);
    va_end(args);

    if (written < 0) {
        if (needs_free) {
            STRING_ALLOCATION_FAILURE = true;
            free(temp);
            return strdup(empty);
        }
        temp[0] = '\0';
        return temp;
    }

    if ((size_t)written >= max_size) {
        temp[max_size - 1] = '\0';
    }

    if (needs_free) {
        char* result = strdup(temp);
        free(temp);
        return result;
    }

    return temp;
}