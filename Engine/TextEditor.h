#pragma once

#include <stdio.h>
#include "raylib.h"
#include "definitions.h"

#define MAX_ROWS 1000
#define MAX_CHARS_PER_ROW 100

#define TEXT_EDITOR_TEXT_SPACING 3.0f

typedef struct TextEditorContext{
    char **text;
    int rowCount;
    char openedFileName[MAX_FILE_NAME];
    int currRow;
    int currCol;
    float cursorBlinkTime;

    bool newLogMessage;
    char logMessages[MAX_LOG_MESSAGES][MAX_LOG_MESSAGE_SIZE];
    LogLevel logMessageLevels[MAX_LOG_MESSAGES];
    int logMessageCount;
}TextEditorContext;

TextEditorContext InitTextEditorContext();

void FreeTextEditorContext(TextEditorContext *txEd);

bool LoadFileInTextEditor(const char *fileName, TextEditorContext *txEd);

void HandleTextEditor(TextEditorContext *txEd, Vector2 mousePos, Rectangle viewportBoundary, RenderTexture2D *viewport, Font font, bool isViewportFocused);