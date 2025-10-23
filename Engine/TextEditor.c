#include "TextEditor.h"

TextEditorContext InitTextEditorContext()
{
    TextEditorContext txEd = {0};

    txEd.text = malloc(MAX_ROWS * sizeof(char *));
    for (int i = 0; i < MAX_ROWS; i++)
    {
        txEd.text[i] = malloc(MAX_CHARS_PER_ROW);
    }

    txEd.rowCount = 0;

    txEd.cursorBlinkTime = 0;

    txEd.isOptionsMenuOpen = false;
    txEd.justClosedMenu = false;

    txEd.isFileOpened = false;

    txEd.font = LoadFontFromMemory(".ttf", JetBrainsMonoNL_ExtraBold_ttf, JetBrainsMonoNL_ExtraBold_ttf_len, FONT_GLYPHS, NULL, 0);

    return txEd;
}

void FreeTextEditorContext(TextEditorContext *txEd)
{
    for (int i = 0; i < MAX_ROWS; i++)
    {
        free(txEd->text[i]);
    }
    free(txEd->text);

    UnloadFont(txEd->font);
}

void ClearTextEditorContext(TextEditorContext *txEd)
{
    txEd->isFileOpened = false;
    txEd->rowCount = 0;
    txEd->openedFilePath[0] = '\0';
    txEd->currRow = 0;
    txEd->currCol = 0;
    txEd->cursorBlinkTime = 0;
    txEd->selectedStart = (Vector2){0, 0};
    txEd->selectedEnd = (Vector2){0, 0};
    txEd->isOptionsMenuOpen = false;
    txEd->justClosedMenu = false;
}

void AddToLogFromTextEditor(TextEditorContext *txEd, char *message, int level)
{
    if (txEd->logMessageCount >= MAX_LOG_MESSAGES)
    {
        return;
    }

    strmac(txEd->logMessages[txEd->logMessageCount], MAX_LOG_MESSAGE_SIZE, "%s", message);
    txEd->logMessageLevels[txEd->logMessageCount] = level;
    txEd->logMessageCount++;
    txEd->newLogMessage = true;
}

bool LoadFileInTextEditor(const char *fileName, TextEditorContext *txEd)
{
    FILE *file = fopen(fileName, "r");
    if (!file)
    {
        return false;
    }

    int line = 0;
    while (line < MAX_ROWS && fgets(txEd->text[line], MAX_CHARS_PER_ROW, file))
    {
        size_t len = strlen(txEd->text[line]);
        if (len > 0 && txEd->text[line][len - 1] == '\n')
        {
            txEd->text[line][len - 1] = '\0';
        }
        line++;
    }

    strmac(txEd->openedFilePath, MAX_FILE_NAME - 1, "%s", fileName);
    txEd->openedFilePath[MAX_FILE_NAME - 1] = '\0';

    fclose(file);

    txEd->rowCount = line;

    txEd->isFileOpened = true;

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
        if (txEd->currRow > 0)
        {
            int prevLen = strlen(txEd->text[txEd->currRow - 1]);
            int currLen = strlen(txEd->text[txEd->currRow]);

            if (prevLen + currLen < MAX_CHARS_PER_ROW)
            {
                strmac(txEd->text[txEd->currRow - 1], MAX_CHARS_PER_ROW, "%s%s",
                       txEd->text[txEd->currRow - 1], txEd->text[txEd->currRow]);
            }

            for (int i = txEd->currRow; i < txEd->rowCount - 1; i++)
            {
                strmac(txEd->text[i], MAX_CHARS_PER_ROW, "%s", txEd->text[i + 1]);
            }

            txEd->text[txEd->rowCount - 1][0] = '\0';
            txEd->rowCount--;
            txEd->currRow--;
            txEd->currCol = prevLen;
        }
        return;
    }

    if (txEd->currCol > 0)
    {
        txEd->currCol--;
        int len = strlen(txEd->text[txEd->currRow]);
        for (int i = txEd->currCol; i < len; i++)
        {
            txEd->text[txEd->currRow][i] = txEd->text[txEd->currRow][i + 1];
        }
    }
}

void AddNewLine(TextEditorContext *txEd)
{
    if (txEd->rowCount < MAX_ROWS)
    {
        for (int i = txEd->rowCount - 1; i >= txEd->currRow; i--)
        {
            strmac(txEd->text[i + 1], MAX_CHARS_PER_ROW, "%s", txEd->text[i]);
        }

        char *currText = txEd->text[txEd->currRow];
        int col = txEd->currCol;
        int len = strlen(currText);

        if (col < len)
        {
            strmac(txEd->text[txEd->currRow + 1], MAX_CHARS_PER_ROW, "%s", currText + col);
            currText[col] = '\0';
        }
        else
        {
            txEd->text[txEd->currRow + 1][0] = '\0';
        }

        txEd->currRow++;
        txEd->currCol = 0;
        txEd->rowCount++;
    }
    else
    {
        AddToLogFromTextEditor(txEd, "Line limit reached! Open file in another text editor{T201}", LOG_LEVEL_ERROR);
    }
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

    int startRow = txEd->selectedStart.x;
    int startCol = txEd->selectedStart.y;
    int endRow = txEd->selectedEnd.x;
    int endCol = txEd->selectedEnd.y;

    if (startRow > endRow || (startCol > endCol && startRow == endRow))
    {
        int temp = startRow;
        startRow = endRow;
        endRow = temp;

        temp = startCol;
        startCol = endCol;
        endCol = temp;
    }

    bool isSelecting = txEd->selectedStart.x != txEd->selectedEnd.x || txEd->selectedStart.y != txEd->selectedEnd.y;

    if (IsKeyDown(KEY_UP) && txEd->currRow != 0)
    {
        if (isSelecting)
        {
            txEd->currRow = startRow;
            txEd->currCol = startCol;

            txEd->selectedStart = (Vector2){startRow, startCol};
            txEd->selectedEnd = txEd->selectedStart;
        }
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

    if (IsKeyDown(KEY_DOWN) && txEd->currRow != txEd->rowCount - 1)
    {
        if (isSelecting)
        {
            txEd->currRow = endRow;
            txEd->currCol = endCol;

            txEd->selectedStart = (Vector2){endRow, endCol};
            txEd->selectedEnd = txEd->selectedStart;
        }
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
        if (isSelecting)
        {
            txEd->currRow = startRow;
            txEd->currCol = startCol;

            txEd->selectedStart = (Vector2){startRow, startCol};
            txEd->selectedEnd = txEd->selectedStart;
        }
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
        if (isSelecting)
        {
            txEd->currRow = endRow;
            txEd->currCol = endCol;

            txEd->selectedStart = (Vector2){endRow, endCol};
            txEd->selectedEnd = txEd->selectedStart;
        }
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

void AddSymbol(TextEditorContext *txEd, char newChar)
{

    if (strlen(txEd->text[txEd->currRow]) < MAX_CHARS_PER_ROW)
    {
        for (int i = strlen(txEd->text[txEd->currRow]); i >= txEd->currCol; i--)
        {
            txEd->text[txEd->currRow][i + 1] = txEd->text[txEd->currRow][i];
        }
        txEd->text[txEd->currRow][txEd->currCol] = newChar;
        txEd->currCol++;
    }
    else
    {
        AddToLogFromTextEditor(txEd, "Character per line limit reached! Go to new line or open file in another text editor{T202}", LOG_LEVEL_ERROR);
    }
}

void DrawSelector(TextEditorContext *txEd, Rectangle viewportBoundary)
{
    int startRow = txEd->selectedStart.x;
    int startCol = txEd->selectedStart.y;
    int endRow = txEd->selectedEnd.x;
    int endCol = txEd->selectedEnd.y;

    if (startRow == endRow && startCol == endCol)
    {
        return;
    }

    if (startRow > endRow || (startCol > endCol && startRow == endRow))
    {
        int temp = startRow;
        startRow = endRow;
        endRow = temp;

        temp = startCol;
        startCol = endCol;
        endCol = temp;
    }

    DrawRectangle(viewportBoundary.x + MeasureTextUntilEx(txEd->font, txEd->text[startRow], startCol, 30, TEXT_EDITOR_TEXT_SPACING) + 50, viewportBoundary.y + startRow * 34 + 60, MeasureTextUntilEx(txEd->font, txEd->text[startRow] + startCol, startRow == endRow ? endCol - startCol : strlen(txEd->text[startRow]), 30, TEXT_EDITOR_TEXT_SPACING) + 5, 30, COLOR_TE_SELECTOR);

    for (int i = startRow + 1; i < endRow; i++)
    {
        DrawRectangle(viewportBoundary.x + 50, viewportBoundary.y + i * 34 + 60, MeasureTextEx(txEd->font, txEd->text[i], 30, TEXT_EDITOR_TEXT_SPACING).x + 5, 30, COLOR_TE_SELECTOR);
    }

    if (startRow != endRow)
    {
        DrawRectangle(viewportBoundary.x + 50, viewportBoundary.y + endRow * 34 + 60, MeasureTextUntilEx(txEd->font, txEd->text[endRow], endCol, 30, TEXT_EDITOR_TEXT_SPACING), 30, COLOR_TE_SELECTOR);
    }
}

void TextEditorDeleteSelected(TextEditorContext *txEd)
{
    int startRow = txEd->selectedStart.x;
    int startCol = txEd->selectedStart.y;
    int endRow = txEd->selectedEnd.x;
    int endCol = txEd->selectedEnd.y;

    if (startRow > endRow || (startCol > endCol && startRow == endRow))
    {
        int temp = startRow;
        startRow = endRow;
        endRow = temp;

        temp = startCol;
        startCol = endCol;
        endCol = temp;
    }

    if (startRow == endRow)
    {
        memmove(txEd->text[startRow] + startCol, txEd->text[startRow] + endCol, strlen(txEd->text[startRow] + endCol) + 1);
    }
    else
    {
        txEd->text[startRow][startCol] = '\0';
        strmac(txEd->text[startRow], MAX_CHARS_PER_ROW, "%s%s", txEd->text[startRow], txEd->text[endRow] + endCol);

        int numRowsToMove = txEd->rowCount - (endRow + 1);
        for (int i = 0; i < numRowsToMove; i++)
        {
            strcpy(txEd->text[startRow + 1 + i], txEd->text[endRow + 1 + i]);
        }

        int rowsDeleted = endRow - startRow;
        for (int i = txEd->rowCount - rowsDeleted; i < txEd->rowCount; i++)
        {
            txEd->text[i][0] = '\0';
        }
        txEd->rowCount -= rowsDeleted;
    }

    txEd->selectedStart = (Vector2){startRow, startCol};
    txEd->selectedEnd = (Vector2){startRow, startCol};
    txEd->currRow = startRow;
    txEd->currCol = startCol;
}

void TextEditorCopy(TextEditorContext *txEd)
{
    int startRow = txEd->selectedStart.x;
    int startCol = txEd->selectedStart.y;
    int endRow = txEd->selectedEnd.x;
    int endCol = txEd->selectedEnd.y;

    if (startRow > endRow || (startCol > endCol && startRow == endRow))
    {
        int temp = startRow;
        startRow = endRow;
        endRow = temp;

        temp = startCol;
        startCol = endCol;
        endCol = temp;
    }

    char *copiedText = NULL;
    char *lastRow = NULL;
    if (startRow == endRow)
    {
        copiedText = strmac(NULL, MAX_CHARS_PER_ROW * MAX_ROWS + 1, "%s", txEd->text[startRow]);
        copiedText[endCol] = '\0';

        copiedText = strmac(NULL, MAX_CHARS_PER_ROW * MAX_ROWS + 1, "%s", copiedText + startCol);
    }
    else
    {
        copiedText = strmac(NULL, MAX_CHARS_PER_ROW * MAX_ROWS + 1, "%s", txEd->text[startRow] + startCol);
        int copiedTextLen = strlen(copiedText);
        copiedText[copiedTextLen] = '\n';
        copiedText[copiedTextLen + 1] = '\0';

        for (int i = startRow + 1; i < endRow; i++)
        {
            copiedText = strmac(NULL, MAX_CHARS_PER_ROW * MAX_ROWS + 1, "%s%s", copiedText, txEd->text[i]);
            int copiedTextLen = strlen(copiedText);
            copiedText[copiedTextLen] = '\n';
            copiedText[copiedTextLen + 1] = '\0';
        }

        lastRow = strmac(NULL, MAX_CHARS_PER_ROW + 1, "%s", txEd->text[endRow]);
        lastRow[endCol] = '\0';

        copiedText = strmac(NULL, MAX_CHARS_PER_ROW * MAX_ROWS + 1, "%s%s", copiedText, lastRow);
    }

    SetClipboardText(copiedText);

    if (copiedText)
    {
        free(copiedText);
    }
    if (lastRow)
    {
        free(lastRow);
    }
}

void TextEditorPaste(TextEditorContext *txEd) {
    if(strlen(GetClipboardText()) > MAX_CHARS_PER_ROW){
        AddToLogFromTextEditor(txEd, "Character per line limit reached! Go to new line or open file in another text editor{T202}", LOG_LEVEL_ERROR);
        return;
    }
    char *clip = strmac(NULL, MAX_CHARS_PER_ROW, "%s", GetClipboardText());
    if (!clip || !clip[0]){
        return;
    }

    if (txEd->selectedStart.x != txEd->selectedEnd.x || txEd->selectedStart.y != txEd->selectedEnd.y) {
        TextEditorDeleteSelected(txEd);
    }

    char *buff = strmac(NULL, MAX_CHARS_PER_ROW, "%s", txEd->text[txEd->currRow]);
    buff[txEd->currCol] = '\0';

    for(int i = strlen(clip) - 1; i >= 0; i--){
        if(clip[i] == '\n'){
            for(int j = i; j < strlen(clip) - 1; j++){
                clip[j] = clip[j + 1];
            }
            clip[strlen(clip) - 1] = '\0';
        }
    }

    if(strlen(TextFormat("%s%s%s", buff, clip, txEd->text[txEd->currRow] + txEd->currCol)) > MAX_CHARS_PER_ROW){
        AddToLogFromTextEditor(txEd, "Character per line limit reached! Go to new line or open file in another text editor{T202}", LOG_LEVEL_ERROR);
        return;
    }

    txEd->text[txEd->currRow] = strmac(NULL, MAX_CHARS_PER_ROW, "%s%s%s", buff, clip, txEd->text[txEd->currRow] + txEd->currCol);

    txEd->currCol += strlen(clip);

    free(buff);
    free(clip);
}

void DrawOptionsMenu(TextEditorContext *txEd, Vector2 mousePos)
{
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        txEd->isOptionsMenuOpen = true;
        txEd->optionsMenuPos = mousePos;
    }

    if (!txEd->isOptionsMenuOpen)
    {
        return;
    }

    DrawRectangleRounded((Rectangle){txEd->optionsMenuPos.x - 60, txEd->optionsMenuPos.y - 95, 80, 95}, 0.2f, 4, COLOR_TE_OPTIONS_MENU);

    DrawTextEx(txEd->font, "Cut", (Vector2){txEd->optionsMenuPos.x - 55, txEd->optionsMenuPos.y - 90}, 24, 1.0f, COLOR_TE_CUT);
    if (CheckCollisionPointRec(mousePos, (Rectangle){txEd->optionsMenuPos.x - 58, txEd->optionsMenuPos.y - 90, 74, 26}))
    {
        txEd->cursor = MOUSE_CURSOR_POINTING_HAND;
        DrawRectangleRounded((Rectangle){txEd->optionsMenuPos.x - 58, txEd->optionsMenuPos.y - 90, 74, 26}, 0.4f, 4, COLOR_TE_OPTIONS_MENU_ITEM_HOVER);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            TextEditorCopy(txEd);
            TextEditorDeleteSelected(txEd);
        }
    }
    DrawTextEx(txEd->font, "Copy", (Vector2){txEd->optionsMenuPos.x - 55, txEd->optionsMenuPos.y - 60}, 24, 1.0f, COLOR_TE_COPY);
    if (CheckCollisionPointRec(mousePos, (Rectangle){txEd->optionsMenuPos.x - 58, txEd->optionsMenuPos.y - 60, 74, 26}))
    {
        txEd->cursor = MOUSE_CURSOR_POINTING_HAND;
        DrawRectangleRounded((Rectangle){txEd->optionsMenuPos.x - 58, txEd->optionsMenuPos.y - 60, 74, 26}, 0.4f, 4, COLOR_TE_OPTIONS_MENU_ITEM_HOVER);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            TextEditorCopy(txEd);
        }
    }
    DrawTextEx(txEd->font, "Paste", (Vector2){txEd->optionsMenuPos.x - 55, txEd->optionsMenuPos.y - 30}, 24, 1.0f, COLOR_TE_PASTE);
    if (CheckCollisionPointRec(mousePos, (Rectangle){txEd->optionsMenuPos.x - 58, txEd->optionsMenuPos.y - 30, 74, 26}))
    {
        txEd->cursor = MOUSE_CURSOR_POINTING_HAND;
        DrawRectangleRounded((Rectangle){txEd->optionsMenuPos.x - 58, txEd->optionsMenuPos.y - 30, 74, 26}, 0.4f, 4, COLOR_TE_OPTIONS_MENU_ITEM_HOVER);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            TextEditorPaste(txEd);
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        txEd->isOptionsMenuOpen = false;
        txEd->justClosedMenu = true;
    }
}

void SaveTextFile(TextEditorContext *txEd, char *filePath)
{
    FILE *file = fopen(filePath, "w");
    if (!file)
        return;

    for (int i = 0; i < txEd->rowCount; i++)
    {
        fprintf(file, "%s\n", txEd->text[i]);
    }

    fclose(file);

    AddToLogFromTextEditor(txEd, "File saved successfully{T300}", LOG_LEVEL_SUCCESS);
}

void KeyboardShortcuts(TextEditorContext *txEd)
{
    if (IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (IsKeyPressed(KEY_X))
        {
            txEd->isOptionsMenuOpen = false;
            TextEditorCopy(txEd);
            TextEditorDeleteSelected(txEd);
        }
        else if (IsKeyPressed(KEY_C))
        {
            txEd->isOptionsMenuOpen = false;
            TextEditorCopy(txEd);
        }
        else if (IsKeyPressed(KEY_V))
        {
            txEd->isOptionsMenuOpen = false;
            TextEditorPaste(txEd);
        }

        if (IsKeyPressed(KEY_S))
        {
            SaveTextFile(txEd, txEd->openedFilePath);
        }
    }
}

void HandleTextEditor(TextEditorContext *txEd, Vector2 mousePos, Rectangle viewportBoundary, RenderTexture2D *viewport, bool isViewportFocused, Font fontArial)
{
    txEd->cursor = MOUSE_CURSOR_IBEAM;

    int x = viewportBoundary.x + 50;
    int y = viewportBoundary.y + 60;

    float frameTime = GetFrameTime();

    txEd->cursorBlinkTime += frameTime;

    if (!txEd->isOptionsMenuOpen && !txEd->justClosedMenu)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isViewportFocused)
        {
            txEd->cursorBlinkTime = 0;

            if (mousePos.y < y)
            {
                txEd->currRow = 0;
            }
            else if (mousePos.y > y + txEd->rowCount * 34)
            {
                txEd->currRow = txEd->rowCount - 1;
            }
            else
            {
                for (int i = 0; i < txEd->rowCount; i++)
                {
                    if (mousePos.y > y + i * 34 && mousePos.y < y + i * 34 + 34)
                    {
                        txEd->currRow = i;
                        break;
                    }
                }
            }

            for (int i = strlen(txEd->text[txEd->currRow]); i >= 0; i--)
            {
                float left = MeasureTextUntilEx(txEd->font, txEd->text[txEd->currRow], i, 30, TEXT_EDITOR_TEXT_SPACING) + x;
                if (mousePos.x >= left)
                {
                    float right = MeasureTextUntilEx(txEd->font, txEd->text[txEd->currRow], i + 1, 30, TEXT_EDITOR_TEXT_SPACING) + x;
                    float mid = (left + right) * 0.5f;
                    txEd->currCol = (mousePos.x < mid) ? i : i + 1;
                    break;
                }
                else if (i == 0)
                {
                    txEd->currCol = 0;
                }
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isViewportFocused)
            {
                txEd->selectedStart = (Vector2){txEd->currRow, txEd->currCol};
            }

            txEd->selectedEnd = (Vector2){txEd->currRow, txEd->currCol};
        }

        static float backspaceTime = 0;
        static bool backspaceHeld = false;

        if (IsKeyDown(KEY_BACKSPACE))
        {
            if (backspaceHeld)
            {
                backspaceTime -= frameTime;
                if (backspaceTime <= 0)
                {
                    if (txEd->selectedStart.x != txEd->selectedEnd.x || txEd->selectedStart.y != txEd->selectedEnd.y)
                    {
                        TextEditorDeleteSelected(txEd);
                    }
                    else
                    {
                        DeleteSymbol(txEd);
                    }
                    txEd->cursorBlinkTime = 0;

                    backspaceTime = 0.05f;
                }
            }
            else
            {
                if (txEd->selectedStart.x != txEd->selectedEnd.x || txEd->selectedStart.y != txEd->selectedEnd.y)
                {
                    TextEditorDeleteSelected(txEd);
                }
                else
                {
                    DeleteSymbol(txEd);
                }
                txEd->cursorBlinkTime = 0;

                backspaceHeld = true;
                backspaceTime = 0.5f;
            }
        }
        else
        {
            backspaceHeld = false;
            backspaceTime = 0.5f;
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            AddNewLine(txEd);
            txEd->cursorBlinkTime = 0;
        }

        while (1)
        {
            char pressed = GetCharPressed();

            if (pressed == 0)
            {
                break;
            }

            AddSymbol(txEd, pressed);
            txEd->selectedStart = txEd->selectedEnd;
        }

        ArrowKeysInput(txEd, frameTime);

        if (txEd->text[txEd->currRow][txEd->currCol - 1] == '\0')
        {
            txEd->currCol--;
        }
    }
    else
    {
        if (txEd->justClosedMenu)
        {
            txEd->justClosedMenu = !IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        }
        txEd->cursor = MOUSE_CURSOR_ARROW;
    }

    KeyboardShortcuts(txEd);

    const char *fileName = GetFileName(txEd->openedFilePath);
    int fileNameSize = MeasureTextEx(fontArial, fileName, 32, 2.0f).x;

    BeginTextureMode(*viewport);
    ClearBackground(GRAY_30);

    DrawTextEx(fontArial, fileName, (Vector2){viewportBoundary.x + 10, viewportBoundary.y + 15}, 32, 2.0f, GRAY_70);

    DrawRectangleRounded((Rectangle){viewportBoundary.x + fileNameSize + 30, viewportBoundary.y + 15, 60, 30}, 0.4f, 4, GRAY_50);
    DrawTextEx(fontArial, "Open", (Vector2){viewportBoundary.x + fileNameSize + 35, viewportBoundary.y + 20}, 18, 2.0f, WHITE);
    if (CheckCollisionPointRec(mousePos, (Rectangle){viewportBoundary.x + fileNameSize + 30, viewportBoundary.y + 15, 60, 30}))
    {
        DrawRectangleRounded((Rectangle){viewportBoundary.x + fileNameSize + 30, viewportBoundary.y + 15, 60, 30}, 0.4f, 4, COlOR_TE_OPEN_BTN_HOVER);
        txEd->cursor = MOUSE_CURSOR_POINTING_HAND;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            OpenFile(txEd->openedFilePath);
        }
    }

    DrawRectangleRounded((Rectangle){viewportBoundary.x + fileNameSize + 100, viewportBoundary.y + 15, 60, 30}, 0.4f, 4, GRAY_50);
    DrawTextEx(fontArial, "Save", (Vector2){viewportBoundary.x + fileNameSize + 108, viewportBoundary.y + 20}, 18, 2.0f, WHITE);
    if (CheckCollisionPointRec(mousePos, (Rectangle){viewportBoundary.x + fileNameSize + 100, viewportBoundary.y + 15, 60, 30}))
    {
        DrawRectangleRounded((Rectangle){viewportBoundary.x + fileNameSize + 100, viewportBoundary.y + 15, 60, 30}, 0.4f, 4, COlOR_TE_OPEN_BTN_HOVER);
        txEd->cursor = MOUSE_CURSOR_POINTING_HAND;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            SaveTextFile(txEd, txEd->openedFilePath);
        }
    }

    if (!DEVELOPER_MODE)
    {
        DrawTextEx(fontArial, "IN DEVELOPMENT", (Vector2){viewportBoundary.x + fileNameSize + 100, viewportBoundary.y + 10}, 40, 2.0f, ORANGE);
    }

    DrawLineEx((Vector2){x - 10, y}, (Vector2){x - 10, y + viewportBoundary.height}, 1.0f, GRAY_40);

    for (int i = 0; i < txEd->rowCount; i++)
    {
        DrawTextEx(txEd->font, TextFormat("%d", i), (Vector2){x - 42 + 5 * !((int)(i / 10) > 0), y + 2}, 26, 1.0f, GRAY);
        DrawTextEx(txEd->font, txEd->text[i], (Vector2){x, y}, 30, TEXT_EDITOR_TEXT_SPACING, COLOR_TE_FONT);
        y += 34;
        DrawRectangleGradientH(viewportBoundary.x, y - 2, viewportBoundary.width, 1, GRAY_50, GRAY_80);
    }

    if (txEd->cursorBlinkTime > 1.5f)
    {
        txEd->cursorBlinkTime = 0;
    }

    float alpha = 255;
    if (txEd->cursorBlinkTime > 0.5f)
    {
        alpha = 255 - (txEd->cursorBlinkTime - 0.5f) * (200 / 1.0f);
    }

    Vector2 start = {MeasureTextUntilEx(txEd->font, txEd->text[txEd->currRow], txEd->currCol, 30, TEXT_EDITOR_TEXT_SPACING) + x + TEXT_EDITOR_TEXT_SPACING / 2, viewportBoundary.y + txEd->currRow * 34 + 58};
    Vector2 end = {start.x, start.y + 34};

    DrawLineEx(start, end, 3.0f, (Color){RAPID_PURPLE.r, RAPID_PURPLE.g, RAPID_PURPLE.b, alpha});

    DrawSelector(txEd, viewportBoundary);

    DrawOptionsMenu(txEd, mousePos);

    EndTextureMode();
}