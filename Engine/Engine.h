#pragma once

#include "raylib.h"
#include "raymath.h"
#include "definitions.h"

#define MAX_UI_ELEMENTS 128
#define MAX_FILE_TOOLTIP_SIZE 512
#define MAX_VARIABLE_TOOLTIP_SIZE 256

#define DOUBLE_CLICK_THRESHOLD 0.3f
#define HOLD_TO_DRAG_THRESHOLD 0.08f

#define UI_LAYER_COUNT 5

#define MIN_WINDOW_WIDTH 300
#define MIN_WINDOW_HEIGHT 300

#define MAX_SETTINGS_LINE 128

typedef enum
{
    UI_ACTION_NO_COLLISION_ACTION,
    UI_ACTION_SAVE_CG,
    UI_ACTION_STOP_GAME,
    UI_ACTION_RUN_GAME,
    UI_ACTION_BUILD_GRAPH,
    UI_ACTION_BACK_FILEPATH,
    UI_ACTION_REFRESH_FILES,
    UI_ACTION_CLOSE_WINDOW,
    UI_ACTION_MINIMIZE_WINDOW,
    UI_ACTION_OPEN_SETTINGS,
    UI_ACTION_MOVE_WINDOW,
    UI_ACTION_RESIZE_BOTTOM_BAR,
    UI_ACTION_RESIZE_SIDE_BAR,
    UI_ACTION_RESIZE_SIDE_BAR_MIDDLE,
    UI_ACTION_OPEN_FILE,
    UI_ACTION_CHANGE_VARS_FILTER,
    UI_ACTION_VAR_TOOLTIP_RUNTIME,
    UI_ACTION_FULLSCREEN_BUTTON_VIEWPORT
} UIAction;

typedef enum
{
    UIRectangle,
    UICircle,
    UILine,
    UIText
} UIElementShape;

typedef enum
{
    VAR_FILTER_ALL,
    VAR_FILTER_NUMBERS,
    VAR_FILTER_STRINGS,
    VAR_FILTER_BOOLS,
    VAR_FILTER_COLORS,
    VAR_FILTER_SPRITES
} VarFilter;

typedef struct LogEntry
{
    char message[MAX_LOG_MESSAGE_SIZE];
    LogLevel level;
} LogEntry;

typedef struct Logs
{
    LogEntry *entries;
    int count;
    int capacity;
} Logs;

typedef struct UIElement
{
    char name[MAX_FILE_PATH];
    UIElementShape shape;
    UIAction type;
    union
    {
        struct
        {
            Vector2 pos;
            Vector2 recSize;
            float roundness;
            int roundSegments;
            Color hoverColor;
        } rect;

        struct
        {
            Vector2 center;
            int radius;
        } circle;

        struct
        {
            Vector2 startPos;
            Vector2 endPos;
            int thickness;
        } line;
    };
    Color color;
    int layer;
    struct
    {
        char string[MAX_FILE_PATH];
        Vector2 textPos;
        int textSize;
        int textSpacing;
        Color textColor;
    } text;

    int valueIndex;
    int fileIndex;
} UIElement;

typedef enum
{
    VIEWPORT_CG_EDITOR,
    VIEWPORT_GAME_SCREEN,
    VIEWPORT_HITBOX_EDITOR,
    VIEWPORT_TEXT_EDITOR
} ViewportMode;

typedef enum
{
    RESIZING_WINDOW_NONE,
    RESIZING_WINDOW_NORTH,
    RESIZING_WINDOW_SOUTH,
    RESIZING_WINDOW_EAST,
    RESIZING_WINDOW_WEST
} WindowResizingButton;

typedef enum
{
    RESIZING_MENU_NONE,
    RESIZING_MENU_BOTTOMBAR,
    RESIZING_MENU_SIDEBAR,
    RESIZING_MENU_SIDEBAR_MIDDLE
} MenuResizingButton;

typedef struct EngineContext
{
    int screenWidth;
    int screenHeight;
    int prevScreenWidth;
    int prevScreenHeight;
    int viewportWidth;
    int viewportHeight;
    float zoom;
    bool isViewportFullscreen;
    bool sideBarHalfSnap;
    int maxScreenWidth;
    int maxScreenHeight;

    int bottomBarHeight;
    int sideBarWidth;
    int sideBarMiddleY;
    UIElement uiElements[MAX_UI_ELEMENTS];
    int uiElementCount;
    int hoveredUIElementIndex;
    bool hasResizedBar;
    bool isEditorOpened;
    bool isViewportFocused;
    bool wasViewportFocusedLastFrame;
    bool isAnyMenuOpen;
    bool isSaveButtonHovered;
    bool isBuildButtonHovered;
    int showSaveWarning;
    bool showSettingsMenu;
    bool shouldCloseWindow;

    RenderTexture2D viewportTex;
    RenderTexture2D uiTex;
    Texture2D resizeButton;
    Texture2D viewportFullscreenButton;
    Texture2D settingsGear;
    Font font;
    ViewportMode viewportMode;

    Vector2 mousePos;

    MenuResizingButton menuResizeButton;

    WindowResizingButton windowResizeButton;
    bool isWindowMoving;

    char *currentPath;
    char *projectPath;
    char *CGFilePath;
    FilePathList files;

    bool isGameRunning;
    bool wasBuilt;
    VarFilter varsFilter;

    Sound saveSound;

    int fps;
    bool delayFrames;
    float autoSaveTimer;

    bool isSoundOn;
    int fpsLimit;
    bool shouldShowFPS;
    bool isAutoSaveON;
    bool shouldHideCursorInGameFullscreen;
    bool isLowSpecModeOn;

    bool isSettingsButtonHovered;

    Logs logs;

    int draggedFileIndex;

    bool openFilesWithRapidEditor;

} EngineContext;

typedef enum
{
    FILE_TYPE_FOLDER = 0,
    FILE_TYPE_CG = 1,
    FILE_TYPE_CONFIG = 2,
    FILE_TYPE_IMAGE = 3,
    FILE_TYPE_OTHER = 4
} FileType;

typedef enum
{
    SETTINGS_MODE_ENGINE,
    SETTINGS_MODE_GAME,
    SETTINGS_MODE_KEYBINDS,
    SETTINGS_MODE_EXPORT,
    SETTINGS_MODE_ABOUT
} SettingsMode;