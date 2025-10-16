#pragma once

#include <stdio.h>
#include "raylib.h"
#include "definitions.h"

#define MAX_ROWS 10000
#define MAX_CHARS_PER_ROW 1000

#define TEXT_EDITOR_TEXT_SPACING 3.0f

typedef struct TextEditorContext{
    bool isFileOpened;

    char **text;
    int rowCount;
    char openedFilePath[MAX_FILE_NAME];
    int currRow;
    int currCol;
    float cursorBlinkTime;

    Vector2 selectedStart;
    Vector2 selectedEnd;

    MouseCursor cursor;

    bool newLogMessage;
    char logMessages[MAX_LOG_MESSAGES][MAX_LOG_MESSAGE_SIZE];
    LogLevel logMessageLevels[MAX_LOG_MESSAGES];
    int logMessageCount;

    bool isOptionsMenuOpen;
    Vector2 optionsMenuPos;
}TextEditorContext;

TextEditorContext InitTextEditorContext();

void FreeTextEditorContext(TextEditorContext *txEd);

void ClearTextEditorContext(TextEditorContext *txEd);

bool LoadFileInTextEditor(const char *fileName, TextEditorContext *txEd);

void HandleTextEditor(TextEditorContext *txEd, Vector2 mousePos, Rectangle viewportBoundary, RenderTexture2D *viewport, Font font, bool isViewportFocused);