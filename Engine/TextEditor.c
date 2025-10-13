#include "TextEditor.h"

TextEditorContext InitTextEditorContext()
{
    TextEditorContext txEd = {0};

    txEd.text = malloc(MAX_LINES * sizeof(char *));
    for (int i = 0; i < MAX_LINES; i++)
    {
        txEd.text[i] = malloc(MAX_CHARS_PER_LINE);
    }

    txEd.isFileOpened = false;

    txEd.lineCount = 0;

    txEd.cursorBlinkTime = 0;

    return txEd;
}

void FreeTextEditorContext(TextEditorContext *txEd)
{
    for (int i = 0; i < MAX_LINES; i++)
    {
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
    while (line < MAX_LINES && fgets(txEd->text[line], MAX_CHARS_PER_LINE, file))
    {
        line++;
    }

    txEd->isFileOpened = true;
    strmac(txEd->openedFileName, MAX_FILE_NAME - 1, "%s", fileName);
    txEd->openedFileName[MAX_FILE_NAME - 1] = '\0';

    fclose(file);

    txEd->lineCount = line;

    for (int i = 0; i < txEd->lineCount; i++)
    {
        for (int j = 0; j < strlen(txEd->text[i]); j++)
        {
            if (txEd->text[i][j] == '\n')
            {
                txEd->text[i][j] = '\0';
            }
        }
    }

    return true;
}

int MeasureTextUntilEx(Font font, const char *text, int index, float fontSize, float spacing)
{
    char temp[1024];
    strncpy(temp, text, index);
    temp[index] = '\0';
    return (int)MeasureTextEx(font, temp, fontSize, spacing).x;
}

void DeleteSymbol(TextEditorContext *txEd)
{
    if (txEd->currCol == 0)
    {
        if (txEd->currRow != 0)
        {
            txEd->currCol = strlen(txEd->text[txEd->currRow - 1]);

            txEd->text[txEd->currRow - 1][strlen(txEd->text[txEd->currRow - 1])] = '\0';
            strmac(txEd->text[txEd->currRow - 1], MAX_CHARS_PER_LINE, "%s%s", txEd->text[txEd->currRow - 1], txEd->text[txEd->currRow]);

            for (int i = txEd->currRow; i < txEd->lineCount - 1; i++)
            {
                strmac(txEd->text[i], MAX_CHARS_PER_LINE, "%s", txEd->text[i + 1]);
            }
            txEd->lineCount--;

            txEd->currRow--;
        }
        return;
    }

    txEd->currCol--;

    for (int i = txEd->currCol; i < strlen(txEd->text[txEd->currRow]) - 1; i++)
    {
        txEd->text[txEd->currRow][i] = txEd->text[txEd->currRow][i + 1];
    }
    txEd->text[txEd->currRow][strlen(txEd->text[txEd->currRow]) - 1] = '\0';
}

void ArrowKeysInput(TextEditorContext *txEd, float frameTime)
{
    static float upTime = 0;
    static float downTime = 0;
    static float leftTime = 0;
    static float rightTime = 0;

    static bool upHeld = false;
    static bool downHeld = false;
    static bool leftHeld = false;
    static bool rightHeld = false;

    if (IsKeyDown(KEY_UP) && txEd->currRow != 0)
    {
        if (upHeld)
        {
            upTime -= frameTime;
            if (upTime <= 0)
            {
                txEd->currRow--;
                txEd->cursorBlinkTime = 0;
                upTime = 0.05f;
            }
        }
        else
        {
            txEd->currRow--;
            txEd->cursorBlinkTime = 0;

            upHeld = true;
            upTime = 0.5f;
        }
    }
    else
    {
        upTime = 0.5f;
        upHeld = false;
    }

    if (IsKeyDown(KEY_DOWN) && txEd->currRow != txEd->lineCount - 1)
    {
        if (downHeld)
        {
            downTime -= frameTime;
            if (downTime <= 0)
            {
                txEd->currRow++;
                txEd->cursorBlinkTime = 0;
                downTime = 0.05f;
            }
        }
        else
        {
            txEd->currRow++;
            txEd->cursorBlinkTime = 0;

            downHeld = true;
            downTime = 0.5f;
        }
    }
    else
    {
        downTime = 0.5f;
        downHeld = false;
    }

    if (IsKeyDown(KEY_LEFT) && txEd->currCol != 0)
    {
        if (leftHeld)
        {
            leftTime -= frameTime;
            if (leftTime <= 0)
            {
                txEd->currCol--;
                txEd->cursorBlinkTime = 0;
                leftTime = 0.05;
            }
        }
        else
        {
            txEd->currCol--;
            txEd->cursorBlinkTime = 0;

            leftHeld = true;
            leftTime = 0.5f;
        }
    }
    else
    {
        leftTime = 0.5f;
        leftHeld = false;
    }

    if (IsKeyDown(KEY_RIGHT) && txEd->currCol != strlen(txEd->text[txEd->currRow]))
    {
        if (rightHeld)
        {
            rightTime -= frameTime;
            if (rightTime <= 0)
            {
                txEd->currCol++;
                txEd->cursorBlinkTime = 0;
                rightTime = 0.05f;
            }
        }
        else
        {
            txEd->currCol++;
            txEd->cursorBlinkTime = 0;

            rightHeld = true;
            rightTime = 0.5f;
        }
    }
    else
    {
        rightTime = 0.5f;
        rightHeld = false;
    }
}

void HandleTextEditor(TextEditorContext *txEd, Vector2 mousePos, Rectangle viewportBoundary, RenderTexture2D *viewport, Font font)
{
    int x = viewportBoundary.x + 40;
    int y = viewportBoundary.y + 60;

    float frameTime = GetFrameTime();

    txEd->cursorBlinkTime += frameTime;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < txEd->lineCount; i++)
        {
            if (mousePos.y > y + i * 34 && mousePos.y < y + i * 34 + 34)
            {
                txEd->currRow = i;
            }
        }

        for (int i = strlen(txEd->text[txEd->currRow]); i >= 0; i--)
        {
            float left = MeasureTextUntilEx(font, txEd->text[txEd->currRow], i, 30, 3.0f) + viewportBoundary.x + 40;
            if (mousePos.x >= left)
            {
                float right = MeasureTextUntilEx(font, txEd->text[txEd->currRow], i + 1, 30, 3.0f) + viewportBoundary.x + 40;
                float mid = (left + right) * 0.5f;
                txEd->currCol = (mousePos.x < mid) ? i : i + 1;
                break;
            }
            else if (i == 0)
            {
                txEd->currCol = 0;
            }
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        DeleteSymbol(txEd);
        txEd->cursorBlinkTime = 0;
    }

    ArrowKeysInput(txEd, frameTime);

    BeginTextureMode(*viewport);
    ClearBackground(GRAY_30);

    if (!DEVELOPER_MODE)
    {
        DrawTextEx(font, "IN DEVELOPMENT", (Vector2){viewportBoundary.x + 10, viewportBoundary.y + 10}, 40, 2.0f, ORANGE);
    }
    else
    {
        DrawTextEx(font, GetFileName(txEd->openedFileName), (Vector2){viewportBoundary.x + 10, viewportBoundary.y + 10}, 32, 2.0f, GRAY_70);
    }

    for (int i = 0; i < txEd->lineCount; i++)
    {
        DrawTextEx(font, TextFormat("%d", i), (Vector2){x - 30, y}, 30, 1.0f, GRAY);
        DrawTextEx(font, txEd->text[i], (Vector2){x, y}, 30, 3.0f, COLOR_TE_FONT);
        y += 34;
        DrawRectangleGradientH(viewportBoundary.x, y - 2, viewportBoundary.width, 1, GRAY_50, GRAY_128);
    }

    if (txEd->cursorBlinkTime <= 1.0f)
    {
        DrawText("|", MeasureTextUntilEx(font, txEd->text[txEd->currRow], txEd->currCol, 30, 3.0f) + x, viewportBoundary.y + txEd->currRow * 34 + 61, 32, WHITE);
    }
    else if (txEd->cursorBlinkTime >= 1.4f)
    {
        txEd->cursorBlinkTime = 0;
    }

    EndTextureMode();
}