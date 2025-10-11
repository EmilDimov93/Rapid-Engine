#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "raylib.h"
#include "local_config.h"
#include "resources/resources.h"

#define FPS_DEFAULT 60
#define FPS_HIGH 140

#define MAX_FILE_NAME 256
#define MAX_FILE_PATH 2048

#define FONT_GLYPHS 128

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
    if (!format || max_size == 0){
        STRING_ALLOCATION_FAILURE = true;
        return (buf) ? buf : strdup("");
    }

    char* temp = buf;
    bool needs_free = false;

    if (!buf) {
        temp = malloc(max_size);
        if (!temp){
            STRING_ALLOCATION_FAILURE = true;
            return strdup("");
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
            return strdup("");
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

const char *AddEllipsis(Font font, const char *text, float fontSize, float maxWidth, bool showEnd)
{
    if (MeasureTextEx(font, text, fontSize, 0).x <= maxWidth){
        return text;
    }

    int len = strlen(text);
    int maxChars = 0;
    char temp[MAX_LITERAL_NODE_FIELD_SIZE];

    float ellipsisWidth = MeasureTextEx(font, "...", fontSize, 0).x;

    for (int i = 1; i <= len; i++)
    {
        if (showEnd){
            strmac(temp, i, "%s", text + len - i);
        }
        else{
            strmac(temp, i, "%.*s", i, text);
        }

        if (MeasureTextEx(font, temp, fontSize, 0).x + ellipsisWidth > maxWidth){
            break;
        }

        maxChars = i;
    }

    if (showEnd)
    {
        return TextFormat("...%s", text + len - maxChars);
    }
    else
    {
        return TextFormat("%.*s...", maxChars, text);
    }
}

#define COLOR_TRANSPARENT (Color){0, 0, 0, 0}

#define GRAY_15 (Color){15, 15, 15, 255}
#define GRAY_28 (Color){28, 28, 28, 255}
#define GRAY_30 (Color){30, 30, 30, 255}
#define GRAY_40 (Color){40, 40, 40, 255}
#define GRAY_50 (Color){50, 50, 50, 255}
#define GRAY_59 (Color){59, 59, 59, 255}
#define GRAY_60 (Color){60, 60, 60, 255}
#define GRAY_70 (Color){70, 70, 70, 255}
#define GRAY_80 (Color){80, 80, 80, 255}
#define GRAY_128 (Color){128, 128, 128, 255}
#define GRAY_150 (Color){150, 150, 150, 255}

#define COLOR_BACKGROUND_BLUR (Color){0, 0, 0, 150}
#define COLOR_SAVE_MENU_DONT_SAVE_BTN (Color){210, 21, 35, 255}
#define COLOR_SETTINGS_MENU_SLIDER_FULL_GREEN (Color){0, 128, 0, 255}
#define COLOR_SETTINGS_MENU_SLIDER_MID_GREEN (Color){65, 179, 89, 255}
#define COLOR_SETTINGS_MENU_DROPDOWN_SELECTED_OPTION (Color){0, 128, 0, 255}
#define COLOR_WARNING_ORANGE (Color){153, 76, 0, 255}
#define COLOR_SETTINGS_MENU_SAVE_BTN_HOVER (Color){255, 255, 255, 40}
#define COLOR_VARS_FILTER_NUMS (Color){64, 159, 189, 255}
#define COLOR_VARS_FILTER_STRINGS (Color){180, 178, 40, 255}
#define COLOR_VARS_FILTER_BOOLS (Color){87, 124, 181, 255}
#define COLOR_VARS_FILTER_COLORS (Color){217, 3, 104, 255}
#define COLOR_VARS_FILTER_SPRITES (Color){3, 206, 164, 255}
#define COLOR_VAR_NUMBER (Color){24, 119, 149, 255}
#define COLOR_VAR_STRING (Color){180, 178, 40, 255}
#define COLOR_VAR_BOOL (Color){27, 64, 121, 255}
#define COLOR_VAR_COLOR (Color){217, 3, 104, 255}
#define COLOR_VAR_SPRITE (Color){3, 206, 164, 255}
#define COLOR_FILE_FOLDER_OUTLINE (Color){102, 102, 11, 255}
#define COLOR_FILE_FOLDER_TEXT (Color){240, 240, 120, 255}
#define COLOR_FILE_CG_OUTLINE (Color){111, 64, 123, 255}
#define COLOR_FILE_CG_TEXT (Color){245, 200, 255, 255}
#define COLOR_FILE_CONFIG_OUTLINE GRAY_80
#define COLOR_FILE_CONFIG_TEXT GRAY_80
#define COLOR_FILE_IMAGE_OUTLINE (Color){44, 88, 148, 255}
#define COLOR_FILE_IMAGE_TEXT (Color){140, 185, 245, 255}
#define COLOR_FILE_OTHER_OUTLINE (Color){124, 123, 120, 255}
#define COLOR_FILE_OTHER_TEXT (Color){242, 240, 235, 255}
#define COLOR_FILE_UNKNOWN GRAY_80
#define COLOR_RESIZE_BUTTON (Color){255, 255, 255, 1}
#define COLOR_COREGRAPH_WATERMARK (Color){255, 255, 255, 51}

#define COLOR_PM_MOVING_DOT (Color){180, 100, 200, 255}
#define COLOR_PM_BACKGROUND (Color){40, 42, 54, 255}
#define COLOR_PM_MAIN_WINDOW_BTNS_HOVER (Color){128, 128, 128, 20}
#define RAPID_PURPLE (Color){202, 97, 255, 255}
#define COLOR_PM_BACK_BTN (Color){70, 70, 70, 150}
#define COLOR_PM_BACK_BTN_HOVER (Color){255, 255, 255, 50}
#define COLOR_PM_SELECTOR_ARROWS (Color){202, 97, 255, 255}
#define COLOR_PM_CREATE_BTN (Color){202, 97, 255, 255}
#define COLOR_PM_CREATE_BTN_HOVER (Color){255, 255, 255, 150}
#define COLOR_PM_CHECKBOX_X (Color){202, 97, 255, 255}
#define COLOR_PM_CHECKBOX_HOVER (Color){255, 255, 255, 100}

#define COLOR_HE_BACKGROUND (Color){80, 0, 90, 100}

#define COLOR_INTP_PAUSE_BLUR (Color){80, 80, 80, 50}

#define COLOR_CGED_WIRE_FLOW (Color){180, 100, 200, 255}
#define COLOR_CGED_WIRE_NUM (Color){24, 119, 149, 255}
#define COLOR_CGED_WIRE_STRING (Color){180, 178, 40, 255}
#define COLOR_CGED_WIRE_BOOL (Color){27, 64, 121, 255}
#define COLOR_CGED_WIRE_COLOR (Color){150, 2, 72, 255}
#define COLOR_CGED_WIRE_SPRITE (Color){3, 206, 164, 255}
#define COLOR_CGED_WIRE_UNKNOWN WHITE
#define COLOR_CGED_WIRE_NEON_BLUE (Color){0, 255, 255, 255}
#define COLOR_CGED_EDIT_HITBOX_BTN_HOVER (Color){255, 255, 255, 100}
#define COLOR_CGED_NODE_HOVER (Color){255, 255, 255, 100}
#define COLOR_CGED_BACKGROUND (Color){40, 42, 54, 255}
#define COLOR_CGED_BACKGROUND_DOT (Color){255, 255, 255, 15}
#define COLOR_CGED_SELECTOR (Color){139, 224, 252, 10}
#define COLOR_CGED_SELECTOR_OUTLINE (Color){53, 179, 252, 255}

