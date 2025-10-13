#pragma once

#include <stdio.h>
#include "raylib.h"
#include "definitions.h"

#define MAX_LINES 1000
#define MAX_CHARS_PER_LINE 1000

typedef struct TextEditorContext{
    char **text;
    int lineCount;
    bool isFileOpened;
    char openedFileName[MAX_FILE_NAME];
    int currRow;
    int currCol;
    float cursorBlinkTime;
}TextEditorContext;

TextEditorContext InitTextEditorContext();

void FreeTextEditorContext(TextEditorContext *txEd);

bool LoadFileInTextEditor(const char *fileName, TextEditorContext *txEd);

void HandleTextEditor(TextEditorContext *txEd, Vector2 mousePos, Rectangle viewportBoundary, RenderTexture2D *viewport, Font font);