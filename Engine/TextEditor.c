#include "TextEditor.h"

TextEditorContext InitTextEditorContext()
{
    TextEditorContext txEd = {0};

    txEd.text = malloc(MAX_LINES * sizeof(char *));
    for(int i = 0; i < MAX_LINES; i++){
        txEd.text[i] = malloc(MAX_CHARS_PER_LINE);
    }

    txEd.isFileOpened = false;

    txEd.lineCount = 0;

    return txEd;
}

void FreeTextEditorContext(TextEditorContext *txEd){
    for(int i = 0; i < MAX_LINES; i++){
        free(txEd->text[i]);
    }
    free(txEd->text);

    txEd->isFileOpened = false;
}

bool LoadFileInTextEditor(const char *fileName, TextEditorContext *txEd)
{
    FILE *file = fopen(fileName, "r");
    if (!file)
    {
        return false;
    }

    int line = 0;
    while (line < MAX_LINES && fgets(txEd->text[line], MAX_CHARS_PER_LINE, file)){
        line++;
    }

    txEd->isFileOpened = true;
    strmac(txEd->openedFileName, MAX_FILE_NAME - 1, "%s", fileName);
    txEd->openedFileName[MAX_FILE_NAME - 1] = '\0';

    fclose(file);

    txEd->lineCount = line;

    return true;
}

void HandleTextEditor(TextEditorContext *txEd, Vector2 mousePos, Rectangle viewportBoundary, RenderTexture2D *viewport, Font font)
{
    BeginTextureMode(*viewport);
    ClearBackground(GRAY_30);

    if(!DEVELOPER_MODE){
        DrawTextEx(font, "IN DEVELOPMENT", (Vector2){viewportBoundary.x + 10, viewportBoundary.y + 10}, 40, 2.0f, ORANGE);
    }
    else{
        DrawTextEx(font, GetFileName(txEd->openedFileName), (Vector2){viewportBoundary.x + 10, viewportBoundary.y + 10}, 32, 2.0f, GRAY_70);
    }

    int x = viewportBoundary.x + 40;
    int y = viewportBoundary.y + 10 + 50;

    for(int i = 0; i < txEd->lineCount; i++){
        DrawTextEx(font, TextFormat("%d", i), (Vector2){x - 30, y}, 30, 1.0f, GRAY);
        DrawTextEx(font, txEd->text[i], (Vector2){x, y}, 30, 3.0f, COLOR_TE_FONT);
        y += 34;
        DrawRectangleGradientH(viewportBoundary.x, y - 2, viewportBoundary.width, 1, GRAY_50, GRAY_128);
    }

    EndTextureMode();
}