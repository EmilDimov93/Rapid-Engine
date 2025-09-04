#pragma once

#include <stdio.h>
#include "raylib.h"
#include "Nodes.h"
#include "definitions.h"

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
    char logMessages[MAX_LOG_MESSAGES][128];
    int logMessageLevels[MAX_LOG_MESSAGES];
    int logMessageCount;

    Vector2 cameraOffset;

    int editingNodeNameIndex;

    int cursor;

    bool hasChanged;
    bool hasChangedInLastFrame;

    int fps;

    float zoom;

    Rectangle viewportBoundary;

    bool createNodeMenuFirstFrame;

    char nodeMenuSearch[64];

    bool shouldOpenHitboxEditor;
    char hitboxEditorFileName[MAX_FILE_NAME];
    int hitboxEditingPinID;
} EditorContext;

EditorContext InitEditorContext(void);

void FreeEditorContext(EditorContext *editor);

void HandleEditor(EditorContext *editor, GraphContext *graph, RenderTexture2D *viewport, Vector2 mousePos, bool draggingDisabled, bool isSecondFrame);