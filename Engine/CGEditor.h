#pragma once

#include <stdio.h>
#include "raylib.h"
#include "Nodes.h"
#include "definitions.h"

#define MAX_KEY_NAME_SIZE 12
#define MAX_SEARCH_BAR_FIELD_SIZE 26

#define MAX_SELECTED_NODES 1000

typedef struct
{
    int screenWidth;
    int screenHeight;

    bool delayFrames;
    bool isFirstFrame;
    bool engineDelayFrames;

    Vector2 mousePos;
    Vector2 rightClickPos;

    Texture2D gearTxt;

    bool isDraggingScreen;
    int draggingNodeIndex;

    bool menuOpen;
    Vector2 menuPosition;
    Vector2 submenuPosition;
    int scrollIndexNodeMenu;
    int hoveredItem;

    Pin lastClickedPin;

    Font font;

    int nodeDropdownFocused;
    int nodeFieldPinFocused;

    bool newLogMessage;
    char logMessages[MAX_LOG_MESSAGES][MAX_LOG_MESSAGE_SIZE];
    LogLevel logMessageLevels[MAX_LOG_MESSAGES];
    int logMessageCount;

    Vector2 cameraOffset; // unused

    int editingNodeNameIndex;

    int cursor;

    bool hasChanged;
    bool hasChangedInLastFrame;

    int fps;

    float zoom;

    Rectangle viewportBoundary;

    bool createNodeMenuFirstFrame;

    char nodeMenuSearch[MAX_SEARCH_BAR_FIELD_SIZE];

    bool shouldOpenHitboxEditor;
    char hitboxEditorFileName[MAX_FILE_NAME];
    int hitboxEditingPinID;

    float nodeGlareTime;

    Node copiedNode;

    bool hasFatalErrorOccurred;

    bool isLowSpecModeOn;

    bool hasDroppedFile;
    char droppedFilePath[MAX_FILE_PATH];

    bool isSelecting;
    int selectedNodes[MAX_SELECTED_NODES];
    int selectedNodesSize;

    GraphContext *graph;
} CGEditorContext;

CGEditorContext InitEditorContext(void);

void FreeEditorContext(CGEditorContext *editor);

void HandleEditor(CGEditorContext *editor, GraphContext *graph, RenderTexture2D *viewport, Vector2 mousePos, bool draggingDisabled, bool isSecondFrame);