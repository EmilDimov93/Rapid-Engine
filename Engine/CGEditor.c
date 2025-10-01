#include "CGEditor.h"
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include "raymath.h"

#define MENU_WIDTH 270
#define MENU_ITEM_HEIGHT 40
#define MENU_VISIBLE_ITEMS 5.5
#define MENU_BORDER_THICKNESS 3
#define SUBMENU_WIDTH 250

void AddToLogFromEditor(CGEditorContext *cgEd, char *message, int level);

CGEditorContext InitEditorContext()
{
    CGEditorContext cgEd = {0};

    cgEd.hasFatalErrorOccurred = false;

    cgEd.lastClickedPin = INVALID_PIN;

    cgEd.scrollIndexNodeMenu = 0;
    cgEd.hoveredItem = 0;

    cgEd.mousePos = (Vector2){0, 0};

    cgEd.draggingNodeIndex = -1;
    cgEd.isDraggingScreen = false;

    cgEd.delayFrames = true;
    cgEd.isFirstFrame = true;
    cgEd.engineDelayFrames = false;

    cgEd.menuOpen = false;
    Vector2 menuPosition = {0, 0};
    Vector2 submenuPosition = {0, 0};

    Image tempImg;
    tempImg = LoadImageFromMemory(".png", node_gear_png, node_gear_png_len);
    cgEd.gearTxt = LoadTextureFromImage(tempImg);
    UnloadImage(tempImg);
    if (cgEd.gearTxt.id == 0)
    {
        cgEd.hasFatalErrorOccurred = true;
        AddToLogFromEditor(&cgEd, "Failed to load texture{C220}", LOG_LEVEL_ERROR);
        return cgEd;
    }

    cgEd.nodeDropdownFocused = -1;
    cgEd.nodeFieldPinFocused = -1;

    cgEd.font = LoadFontFromMemory(".ttf", arialbd_ttf, arialbd_ttf_len, 256, NULL, 0);
    if (cgEd.font.texture.id == 0)
    {
        cgEd.hasFatalErrorOccurred = true;
        AddToLogFromEditor(&cgEd, "Failed to load font{C221}", LOG_LEVEL_ERROR);
        return cgEd;
    }

    cgEd.newLogMessage = false;

    cgEd.cameraOffset = (Vector2){0, 0};

    cgEd.editingNodeNameIndex = -1;

    cgEd.hasChanged = false;
    cgEd.hasChangedInLastFrame = false;

    cgEd.createNodeMenuFirstFrame = true;

    cgEd.zoom = 1.0f;

    cgEd.nodeGlareTime = 0;

    cgEd.copiedNode.id = -1;

    cgEd.isLowSpecModeOn = false;

    return cgEd;
}

void FreeEditorContext(CGEditorContext *cgEd)
{
    UnloadTexture(cgEd->gearTxt);

    UnloadFont(cgEd->font);

    if (cgEd->graph)
    {
        FreeGraphContext(cgEd->graph);
    }
}

void AddToLogFromEditor(CGEditorContext *cgEd, char *message, int level)
{
    if (cgEd->logMessageCount >= MAX_LOG_MESSAGES)
    {
        return;
    }

    strmac(cgEd->logMessages[cgEd->logMessageCount], MAX_LOG_MESSAGE_SIZE, "%s", message);
    cgEd->logMessageLevels[cgEd->logMessageCount] = level;
    cgEd->logMessageCount++;
    cgEd->newLogMessage = true;
}

void DrawBackgroundGrid(CGEditorContext *cgEd, int gridSpacing, RenderTexture2D dot)
{
    static Vector2 offset;
    if (cgEd->isDraggingScreen)
    {
        offset = Vector2Subtract(offset, Vector2Scale(GetMouseDelta(), 1.0f / (cgEd->zoom * cgEd->zoom)));
    }
    gridSpacing = (gridSpacing / cgEd->zoom > 1) ? gridSpacing / cgEd->zoom : 1;

    offset = Vector2Clamp(offset, (Vector2){-VIEWPORT_BORDER, -VIEWPORT_BORDER}, (Vector2){VIEWPORT_BORDER, VIEWPORT_BORDER});

    float worldLeft = offset.x;
    float worldTop = offset.y;
    float worldRight = offset.x + cgEd->screenWidth / cgEd->zoom;
    float worldBottom = offset.y + cgEd->screenHeight / cgEd->zoom;

    int startX = ((int)worldLeft / gridSpacing) * gridSpacing - gridSpacing;
    int startY = ((int)worldTop / gridSpacing) * gridSpacing - gridSpacing;
    int endX = ((int)worldRight / gridSpacing) * gridSpacing + gridSpacing;
    int endY = ((int)worldBottom / gridSpacing) * gridSpacing + gridSpacing;

    for (int y = startY; y <= endY; y += gridSpacing)
    {
        int row = y / gridSpacing;
        for (int x = startX; x <= endX; x += gridSpacing)
        {
            float drawX = x + (row % 2) * (gridSpacing / 2);
            float drawY = (float)y;

            float screenX = (drawX - offset.x) * cgEd->zoom;
            float screenY = (drawY - offset.y) * cgEd->zoom;

            DrawTextureRec(dot.texture, (Rectangle){0, 0, (float)dot.texture.width, (float)-dot.texture.height}, (Vector2){screenX, screenY}, COLOR_CGED_BACKGROUND_DOT);
        }
    }
}

void DrawCurvedWire(Vector2 outputPos, Vector2 inputPos, float thickness, Color color, bool isLowSpecModeOn)
{
    const float inputOffset = 12.0f;
    const float outputOffset = 17.0f;

    float distance = fabsf(inputPos.x - outputPos.x);
    float controlOffset = distance * 0.5f;

    DrawLineEx(outputPos, (Vector2){outputPos.x + outputOffset, outputPos.y}, thickness, color);
    inputPos.x -= inputOffset;
    outputPos.x += outputOffset;

    Vector2 p0 = outputPos;
    Vector2 p1 = {outputPos.x + controlOffset, outputPos.y};
    Vector2 p2 = {inputPos.x - controlOffset, inputPos.y};
    Vector2 p3 = inputPos;

    const int segments = isLowSpecModeOn ? 12 : Clamp((int)(distance / 8.0f), 12, 64);

    Vector2 points[65];
    points[0] = p0;
    for (int i = 1; i <= segments; i++)
    {
        float t = (float)i / segments;
        float u = 1.0f - t;
        points[i] = (Vector2){
            u * u * u * p0.x + 3 * u * u * t * p1.x + 3 * u * t * t * p2.x + t * t * t * p3.x,
            u * u * u * p0.y + 3 * u * u * t * p1.y + 3 * u * t * t * p2.y + t * t * t * p3.y};
    }

    if (!isLowSpecModeOn)
    {
        for (int glow = 3; glow > 0; glow--)
        {
            float glowThickness = thickness + glow * 1.2f;
            Color glowColor = color;
            glowColor.a = 100;
            for (int i = 1; i <= segments; i++)
            {
                DrawLineEx(points[i - 1], points[i], glowThickness, glowColor);
            }
        }
    }

    for (int i = 1; i <= segments; i++)
    {
        DrawLineEx(points[i - 1], points[i], thickness, color);
    }

    DrawLineEx(inputPos, (Vector2){inputPos.x + inputOffset, inputPos.y}, thickness, color);
}

void HandleVarNameTextBox(CGEditorContext *cgEd, Rectangle bounds, char *text, int index, GraphContext *graph)
{
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
    {
        cgEd->hasChangedInLastFrame = true;
        const char *clipboard = GetClipboardText();
        if (clipboard)
        {
            strmac(text, MAX_VARIABLE_NAME_SIZE, "%s%s", text, clipboard);
        }
    }

    bounds.width = MeasureTextEx(cgEd->font, text, 16, 2).x + 25;

    DrawRectangleRounded((Rectangle){bounds.x, bounds.y - 20, 56, bounds.height}, 0.4f, 4, DARKGRAY);
    DrawTextEx(cgEd->font, "Name:", (Vector2){bounds.x + 4, bounds.y - 16}, 14, 2, WHITE);

    DrawRectangleRounded(bounds, 0.6f, 4, GRAY);
    DrawRectangleRoundedLinesEx(bounds, 0.6f, 4, 2, DARKGRAY);

    bool showCursor = ((int)(GetTime() * 2) % 2) == 0;
    char buffer[MAX_VARIABLE_NAME_SIZE];
    strmac(buffer, MAX_VARIABLE_NAME_SIZE, "%s%s", text, showCursor ? "_" : " ");
    DrawTextEx(cgEd->font, buffer, (Vector2){bounds.x + 5, bounds.y + 8}, 16, 2, BLACK);

    int key = GetCharPressed();

    bool hasNameChanged = false;

    if (key > 0)
    {
        int len = strlen(text);
        if (len < MAX_VARIABLE_NAME_SIZE - 1 && key >= 32 && key <= 125)
        {
            text[len] = (char)key;
            text[len + 1] = '\0';
            hasNameChanged = true;
            cgEd->hasChangedInLastFrame = true;
        }
    }

    static float backspaceTimer = 0;
    static bool backspaceHeld = false;
    if (IsKeyDown(KEY_BACKSPACE))
    {
        if (!backspaceHeld)
        {
            int len = strlen(text);
            if (len > 0)
            {
                text[len - 1] = '\0';
                hasNameChanged = true;
                cgEd->hasChangedInLastFrame = true;
            }
            backspaceTimer = 0.5f;
            backspaceHeld = true;
        }
        else
        {
            backspaceTimer -= GetFrameTime();
            if (backspaceTimer <= 0)
            {
                int len = strlen(text);
                if (len > 0)
                {
                    text[len - 1] = '\0';
                    hasNameChanged = true;
                    cgEd->hasChangedInLastFrame = true;
                }
                backspaceTimer = 0.05f;
            }
        }
    }
    else
    {
        backspaceHeld = false;
    }

    if (hasNameChanged)
    {
        for (int i = 0; i < graph->variablesCount; i++)
        {
            free(graph->variables[i]);
        }
        free(graph->variables);
        free(graph->variableTypes);

        int newCount = 1;
        for (int i = 0; i < graph->nodeCount; i++)
        {
            if (graph->nodes[i].type == NODE_CREATE_NUMBER ||
                graph->nodes[i].type == NODE_CREATE_STRING ||
                graph->nodes[i].type == NODE_CREATE_BOOL ||
                graph->nodes[i].type == NODE_CREATE_COLOR ||
                graph->nodes[i].type == NODE_CREATE_SPRITE)
            {
                newCount++;
            }
        }

        graph->variables = malloc(sizeof(char *) * newCount);
        graph->variableTypes = malloc(sizeof(NodeType) * newCount);

        graph->variables[0] = strmac(NULL, 5, "NONE");
        graph->variableTypes[0] = NODE_UNKNOWN;

        int idx = 1;
        for (int i = 0; i < graph->nodeCount; i++)
        {
            if (graph->nodes[i].type == NODE_CREATE_NUMBER || graph->nodes[i].type == NODE_CREATE_STRING || graph->nodes[i].type == NODE_CREATE_BOOL || graph->nodes[i].type == NODE_CREATE_COLOR || graph->nodes[i].type == NODE_CREATE_SPRITE)
            {
                graph->variables[idx] = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "%s", graph->nodes[i].name);
                graph->variableTypes[idx] = graph->nodes[i].type;
                idx++;
            }
        }

        graph->variablesCount = newCount;
    }
}

void HandleLiteralNodeField(CGEditorContext *cgEd, GraphContext *graph, int currPinIndex)
{
    PinType type = graph->pins[currPinIndex].type;
    int limit = 0;
    switch (type)
    {
    case PIN_FIELD_NUM:
        limit = 90;
        break;
    case PIN_FIELD_STRING:
        limit = 150;
        break;
    case PIN_FIELD_BOOL:
        limit = 100;
        break;
    case PIN_FIELD_COLOR:
        limit = 120;
        break;
    default:
        break;
    }

    Rectangle textbox = {
        graph->pins[currPinIndex].position.x - 6,
        graph->pins[currPinIndex].position.y - 10,
        limit,
        24};

    float textWidth = MeasureTextEx(cgEd->font, graph->pins[currPinIndex].textFieldValue, 20, 0).x;
    if (cgEd->nodeFieldPinFocused == currPinIndex)
    {
        textWidth += MeasureTextEx(cgEd->font, "_", 20, 0).x;
    }
    float boxWidth = (textWidth + 10 > limit) ? limit : textWidth + 10;
    if (boxWidth < 25)
    {
        boxWidth = 25;
    }
    textbox.width = boxWidth;

    bool isFieldHovered = CheckCollisionPointRec(cgEd->mousePos, textbox);

    if (isFieldHovered && type != PIN_FIELD_BOOL)
    {
        cgEd->cursor = MOUSE_CURSOR_IBEAM;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (isFieldHovered)
        {
            if (type == PIN_FIELD_BOOL)
            {
                cgEd->hasChangedInLastFrame = true;
                if (strcmp(graph->pins[currPinIndex].textFieldValue, "false") == 0 || graph->pins[currPinIndex].textFieldValue[0] == '\0')
                {
                    strmac(graph->pins[currPinIndex].textFieldValue, 5, "true");
                }
                else
                {
                    strmac(graph->pins[currPinIndex].textFieldValue, 6, "false");
                }
            }
            else
            {
                cgEd->nodeFieldPinFocused = currPinIndex;
            }
        }
        else if (cgEd->nodeFieldPinFocused == currPinIndex)
        {
            switch (type)
            {
            case PIN_FIELD_NUM:
                if (graph->pins[currPinIndex].textFieldValue[0] == '\0')
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, 2, "0");
                }
                break;
            case PIN_FIELD_STRING:
                if (graph->pins[currPinIndex].textFieldValue[0] == '\0')
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, 1, "");
                }
                break;
            case PIN_FIELD_COLOR:
                if (strlen(graph->pins[currPinIndex].textFieldValue) != 8)
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, 9, "00000000");
                }
                break;
            default:
                break;
            }
            cgEd->nodeFieldPinFocused = -1;
        }
    }

    Color textboxColor = (cgEd->nodeFieldPinFocused == currPinIndex) ? LIGHTGRAY : (isFieldHovered ? WHITE : GRAY);

    DrawRectangleRec(textbox, textboxColor);
    DrawRectangleLinesEx(textbox, 1, WHITE);

    const char *originalText = graph->pins[currPinIndex].textFieldValue;
    const char *text = originalText;

    if (boxWidth == limit)
    {
        text = AddEllipsis(cgEd->font, originalText, 20, limit - 10, cgEd->nodeFieldPinFocused == currPinIndex);
    }

    if (cgEd->nodeFieldPinFocused == currPinIndex)
    {
        static float blinkTime = 0;
        blinkTime += GetFrameTime();
        char blinking[MAX_LITERAL_NODE_FIELD_SIZE];
        strmac(blinking, MAX_LITERAL_NODE_FIELD_SIZE, "%s%c", text, (fmodf(blinkTime, 1.0f) < 0.5f) ? '_' : '\0');
        DrawTextEx(cgEd->font, cgEd->nodeFieldPinFocused == currPinIndex ? blinking : text, (Vector2){textbox.x + 5, textbox.y + 4}, 20, 0, BLACK);
    }
    else
    {
        DrawTextEx(cgEd->font, text, (Vector2){textbox.x + 5, textbox.y + 4}, 20, 0, BLACK);
    }

    if (cgEd->nodeFieldPinFocused == currPinIndex)
    {
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
        {
            const char *clipboard = GetClipboardText();
            if (clipboard)
            {
                bool validClipboard = true;
                for (int i = 0; i < strlen(clipboard); i++)
                {
                    bool validSymbol =
                        (type == PIN_FIELD_NUM && clipboard[i] >= '0' && clipboard[i] <= '9') ||
                        (type == PIN_FIELD_STRING && clipboard[i] >= 32 && clipboard[i] <= 126) ||
                        (type == PIN_FIELD_COLOR &&
                         ((clipboard[i] >= '0' && clipboard[i] <= '9') || (clipboard[i] >= 'a' && clipboard[i] <= 'f') || (clipboard[i] >= 'A' && clipboard[i] <= 'F')));

                    if (!validSymbol)
                    {
                        validClipboard = false;
                        break;
                    }
                }

                if (validClipboard)
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, MAX_VARIABLE_NAME_SIZE, "%s%s", graph->pins[currPinIndex].textFieldValue, clipboard);
                }
            }
        }

        int key = GetCharPressed();
        while (key > 0)
        {
            int len = strlen(graph->pins[currPinIndex].textFieldValue);

            bool validKey =
                (type == PIN_FIELD_NUM && key >= '0' && key <= '9') ||
                (type == PIN_FIELD_STRING && key >= 32 && key <= 126) ||
                (type == PIN_FIELD_COLOR &&
                 ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f') || (key >= 'A' && key <= 'F')));

            if (validKey)
            {
                cgEd->hasChangedInLastFrame = true;
                if (type == PIN_FIELD_COLOR && len >= 8)
                {
                    key = GetCharPressed();
                    continue;
                }

                graph->pins[currPinIndex].textFieldValue[len] = (char)key;
                graph->pins[currPinIndex].textFieldValue[len + 1] = '\0';
            }
            else if (type == PIN_FIELD_NUM)
            {
                if (key == KEY_PERIOD && !graph->pins[currPinIndex].isNumFloat)
                {
                    cgEd->hasChangedInLastFrame = true;
                    graph->pins[currPinIndex].textFieldValue[len] = (char)key;
                    graph->pins[currPinIndex].textFieldValue[len + 1] = '\0';
                    graph->pins[currPinIndex].isNumFloat = true;
                }
                if (key == KEY_MINUS && len == 0)
                {
                    graph->pins[currPinIndex].textFieldValue[len] = (char)key;
                }
            }

            key = GetCharPressed();
        }

        static float backspaceTimer = 0;
        static bool backspaceHeld = false;
        if (IsKeyDown(KEY_BACKSPACE))
        {
            cgEd->hasChangedInLastFrame = true;
            if (!backspaceHeld)
            {
                size_t len = strlen(graph->pins[currPinIndex].textFieldValue);
                if (len > 0)
                {
                    if (type == PIN_FIELD_NUM &&
                        graph->pins[currPinIndex].textFieldValue[len - 1] == 46)
                    {
                        graph->pins[currPinIndex].isNumFloat = false;
                    }
                    graph->pins[currPinIndex].textFieldValue[len - 1] = '\0';
                }
                backspaceTimer = 0.5f;
                backspaceHeld = true;
            }
            else
            {
                backspaceTimer -= GetFrameTime();
                if (backspaceTimer <= 0)
                {
                    size_t len = strlen(graph->pins[currPinIndex].textFieldValue);
                    if (len > 0)
                    {
                        if (type == PIN_FIELD_NUM &&
                            graph->pins[currPinIndex].textFieldValue[len - 1] == 46)
                        {
                            graph->pins[currPinIndex].isNumFloat = false;
                        }
                        graph->pins[currPinIndex].textFieldValue[len - 1] = '\0';
                    }
                    backspaceTimer = 0.05f;
                }
            }
        }
        else
        {
            backspaceHeld = false;
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            switch (type)
            {
            case PIN_FIELD_NUM:
                if (graph->pins[currPinIndex].textFieldValue[0] == '\0')
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, 2, "0");
                }
                break;
            case PIN_FIELD_STRING:
                if (graph->pins[currPinIndex].textFieldValue[0] == '\0')
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, 1, "");
                }
                break;
            case PIN_FIELD_COLOR:
                if (strlen(graph->pins[currPinIndex].textFieldValue) != 8)
                {
                    cgEd->hasChangedInLastFrame = true;
                    strmac(graph->pins[currPinIndex].textFieldValue, 9, "00000000");
                }
                break;
            default:
                break;
            }
            cgEd->nodeFieldPinFocused = -1;
        }
    }
}

void HandleKeyNodeField(CGEditorContext *cgEd, GraphContext *graph, int currPinIndex)
{
    Rectangle textbox = {
        graph->pins[currPinIndex].position.x - 6,
        graph->pins[currPinIndex].position.y - 10,
        cgEd->nodeFieldPinFocused == currPinIndex ? 110 : MeasureTextEx(cgEd->font, GetKeyboardKeyName(graph->pins[currPinIndex].pickedOption), 20, 0).x + 10,
        24};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(cgEd->mousePos, textbox))
        {
            cgEd->nodeFieldPinFocused = currPinIndex;
        }
        else if (cgEd->nodeFieldPinFocused == currPinIndex)
        {
            cgEd->nodeFieldPinFocused = -1;
        }
    }

    DrawRectangleRec(textbox, (cgEd->nodeFieldPinFocused == currPinIndex) ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(textbox, 1, WHITE);

    if (cgEd->nodeFieldPinFocused == currPinIndex)
    {
        static float blinkTime = 0;
        blinkTime += GetFrameTime();
        if (fmodf(blinkTime, 1.0f) < 0.6f)
        {
            char blinking[MAX_KEY_NAME_SIZE];
            strmac(blinking, MAX_KEY_NAME_SIZE, "Press a key");
            DrawTextEx(cgEd->font, blinking, (Vector2){textbox.x + 5, textbox.y + 4}, 20, 0, BLACK);
        }
    }
    else
    {
        DrawTextEx(cgEd->font, GetKeyboardKeyName(graph->pins[currPinIndex].pickedOption), (Vector2){textbox.x + 5, textbox.y + 4}, 20, 0, BLACK);
    }

    if (cgEd->nodeFieldPinFocused == currPinIndex)
    {
        for (int key = 0; key <= KEY_KB_MENU; key++)
        {
            if (IsKeyPressed(key))
            {
                cgEd->hasChangedInLastFrame = true;
                graph->pins[currPinIndex].pickedOption = key;
                cgEd->nodeFieldPinFocused = -1;
                break;
            }
        }
    }
}

void HandleDropdownMenu(GraphContext *graph, int currPinIndex, int hoveredNodeIndex, int currNodeIndex, CGEditorContext *cgEd)
{
    DropdownOptionsByPinType options;
    if (graph->pins[currPinIndex].type == PIN_VARIABLE || graph->pins[currPinIndex].type == PIN_SPRITE_VARIABLE)
    {
        options.boxWidth = 100;
        options.optionsCount = graph->variablesCount;
        options.options = graph->variables;

        if (options.optionsCount == 0)
        {
            static char *noVars[] = {"No variables"};
            options.options = noVars;
            options.optionsCount = 1;
        }
    }
    else
    {
        options = getPinDropdownOptionsByType(graph->pins[currPinIndex].type);
    }

    if (graph->pins[currPinIndex].pickedOption >= options.optionsCount)
    {
        graph->pins[currPinIndex].pickedOption = 0;
    }

    Rectangle dropdown = {graph->pins[currPinIndex].position.x - 6, graph->pins[currPinIndex].position.y - 10, options.boxWidth, 24};

    DrawRectangleRec(dropdown, GRAY);
    const char *text = AddEllipsis(cgEd->font, options.options[graph->pins[currPinIndex].pickedOption], 20, options.boxWidth - 15, false);
    DrawTextEx(cgEd->font, text, (Vector2){(graph->pins[currPinIndex].type == PIN_VARIABLE || graph->pins[currPinIndex].type == PIN_SPRITE_VARIABLE) ? dropdown.x + 20 : dropdown.x + 3, dropdown.y + 3}, 20, 0, BLACK);
    DrawRectangleLinesEx(dropdown, 1, WHITE);
    if (graph->pins[currPinIndex].type == PIN_VARIABLE || graph->pins[currPinIndex].type == PIN_SPRITE_VARIABLE)
    {
        Color varTypeColor;
        PinType varType = PIN_UNKNOWN_VALUE;
        switch (graph->variableTypes[graph->pins[currPinIndex].pickedOption])
        {
        case NODE_CREATE_NUMBER:
            varTypeColor = COLOR_VAR_NUMBER;
            varType = PIN_NUM;
            break;
        case NODE_CREATE_STRING:
            varTypeColor = COLOR_VAR_STRING;
            varType = PIN_STRING;
            break;
        case NODE_CREATE_BOOL:
            varTypeColor = COLOR_VAR_BOOL;
            varType = PIN_BOOL;
            break;
        case NODE_CREATE_COLOR:
            varTypeColor = COLOR_VAR_COLOR;
            varType = PIN_COLOR;
            break;
        case NODE_CREATE_SPRITE:
            varTypeColor = COLOR_VAR_SPRITE;
            varType = PIN_SPRITE;
            break;
        default:
            varTypeColor = LIGHTGRAY;
        }
        if (graph->nodes[currNodeIndex].type == NODE_GET_VARIABLE)
        {
            graph->pins[FindPinIndexByID(graph, graph->nodes[currNodeIndex].outputPins[0])].type = varType;
        }
        else if (graph->nodes[currNodeIndex].type == NODE_SET_VARIABLE)
        {
            graph->pins[FindPinIndexByID(graph, graph->nodes[currNodeIndex].inputPins[2])].type = varType;
        }
        DrawCircle(graph->pins[currPinIndex].position.x + 4, graph->pins[currPinIndex].position.y, 6, varTypeColor);
    }

    bool mouseOnDropdown = CheckCollisionPointRec(cgEd->mousePos, dropdown);
    bool mouseOnOptions = false;
    if (cgEd->nodeDropdownFocused == currPinIndex)
    {
        for (int j = 0; j < options.optionsCount; j++)
        {
            Rectangle option = {dropdown.x, dropdown.y - (j + 1) * 30, dropdown.width, 30};
            if (CheckCollisionPointRec(cgEd->mousePos, option))
            {
                mouseOnOptions = true;
                break;
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (cgEd->nodeDropdownFocused == currPinIndex)
        {
            if (mouseOnDropdown)
                cgEd->nodeDropdownFocused = -1;
            else if (!mouseOnOptions)
                cgEd->nodeDropdownFocused = -1;
        }
        else if (mouseOnDropdown)
        {
            cgEd->nodeDropdownFocused = currPinIndex;
        }
    }

    if (cgEd->nodeDropdownFocused == currPinIndex)
    {
        cgEd->delayFrames = true;
        hoveredNodeIndex = -1;
        int displayedVarsCounter = 0;
        for (int j = 0; j < options.optionsCount; j++)
        {
            displayedVarsCounter++;
            if ((graph->pins[currPinIndex].type == PIN_SPRITE_VARIABLE && graph->variableTypes[j] != NODE_CREATE_SPRITE) && j != 0)
            {
                displayedVarsCounter--;
                continue;
            }
            Rectangle option = {dropdown.x, dropdown.y - displayedVarsCounter * 30, dropdown.width, 30};
            DrawRectangleRec(option, RAYWHITE);
            const char *text = AddEllipsis(cgEd->font, options.options[j], 20, options.boxWidth - 15, false);
            DrawTextEx(cgEd->font, text, (Vector2){(graph->pins[currPinIndex].type == PIN_VARIABLE || graph->pins[currPinIndex].type == PIN_SPRITE_VARIABLE) ? option.x + 20 : option.x + 3, option.y + 3}, 20, 0, BLACK);
            DrawRectangleLinesEx(option, 1, DARKGRAY);

            if (graph->pins[currPinIndex].type == PIN_VARIABLE || graph->pins[currPinIndex].type == PIN_SPRITE_VARIABLE)
            {
                Color varTypeColor;
                switch (graph->variableTypes[j])
                {
                case NODE_CREATE_NUMBER:
                    varTypeColor = COLOR_VAR_NUMBER;
                    break;
                case NODE_CREATE_STRING:
                    varTypeColor = COLOR_VAR_STRING;
                    break;
                case NODE_CREATE_BOOL:
                    varTypeColor = COLOR_VAR_BOOL;
                    break;
                case NODE_CREATE_COLOR:
                    varTypeColor = COLOR_VAR_COLOR;
                    break;
                case NODE_CREATE_SPRITE:
                    varTypeColor = COLOR_VAR_SPRITE;
                    break;
                default:
                    varTypeColor = LIGHTGRAY;
                }
                DrawCircle(option.x + 10, option.y + 12, 6, varTypeColor);
            }

            if (CheckCollisionPointRec(cgEd->mousePos, option) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                graph->pins[currPinIndex].pickedOption = j;
                cgEd->nodeDropdownFocused = -1;
                cgEd->hasChangedInLastFrame = true;
            }
        }
    }
}

void DrawNodes(CGEditorContext *cgEd, GraphContext *graph)
{
    if (graph->nodeCount == 0)
    {
        return;
    }

    for (int i = 0; i < graph->linkCount; i++)
    {
        Vector2 inputPinPosition;
        Vector2 outputPinPosition;
        bool isInputPosSet = false;
        bool isOutputPosSet = false;
        bool isInputFlow = false;
        bool isOutputFlow = false;
        PinType linkType;
        for (int j = 0; j < graph->pinCount; j++)
        {
            if (graph->links[i].inputPinID == graph->pins[j].id)
            {
                inputPinPosition = graph->pins[j].position;
                isInputPosSet = true;
                isInputFlow = (graph->pins[j].type == PIN_FLOW);

                if (graph->pins[j].type != PIN_ANY_VALUE)
                {
                    linkType = graph->pins[j].type;
                }
            }
            else if (graph->links[i].outputPinID == graph->pins[j].id)
            {
                outputPinPosition = graph->pins[j].position;
                isOutputPosSet = true;
                isOutputFlow = (graph->pins[j].type == PIN_FLOW);

                if (graph->pins[j].type != PIN_ANY_VALUE)
                {
                    linkType = graph->pins[j].type;
                }
            }
        }
        bool isFlowLink = isInputFlow && isOutputFlow;
        if (isInputPosSet && isOutputPosSet)
        {
            Color wireColor;
            switch (linkType)
            {
            case PIN_FLOW:
                wireColor = COLOR_CGED_WIRE_FLOW;
                break;
            case PIN_NUM:
                wireColor = COLOR_CGED_WIRE_NUM;
                break;
            case PIN_STRING:
                wireColor = COLOR_CGED_WIRE_STRING;
                break;
            case PIN_BOOL:
                wireColor = COLOR_CGED_WIRE_BOOL;
                break;
            case PIN_COLOR:
                wireColor = COLOR_CGED_WIRE_COLOR;
                break;
            case PIN_SPRITE:
                wireColor = COLOR_CGED_WIRE_SPRITE;
                break;
            default:
                wireColor = COLOR_CGED_WIRE_UNKNOWN;
            }
            DrawCurvedWire(outputPinPosition, inputPinPosition, 2.0f + 1.0f / cgEd->zoom + isFlowLink, wireColor, cgEd->isLowSpecModeOn);
        }
        else
        {
            AddToLogFromEditor(cgEd, "Error drawing connection{C110}", LOG_LEVEL_WARNING);
        }
    }

    int hoveredNodeIndex = -1;
    int nodeToDelete = -1;
    static Rectangle textBoxRect = {0};

    for (int i = 0; i < graph->nodeCount; i++)
    {
        float x = graph->nodes[i].position.x;
        float y = graph->nodes[i].position.y;
        float width = getNodeInfoByType(graph->nodes[i].type, WIDTH);
        float height = getNodeInfoByType(graph->nodes[i].type, HEIGHT);
        float roundness = 0.2f;
        float segments = 8;
        int glareOffset = 0;

        if (CheckCollisionPointRec(cgEd->mousePos, (Rectangle){graph->nodes[i].position.x, graph->nodes[i].position.y, getNodeInfoByType(graph->nodes[i].type, WIDTH), getNodeInfoByType(graph->nodes[i].type, HEIGHT)}))
        {
            hoveredNodeIndex = i;
            cgEd->nodeGlareTime += GetFrameTime();
            glareOffset = (int)(sinf(cgEd->nodeGlareTime * 6.0f) * 30);

            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            {
                nodeToDelete = graph->nodes[i].id;
            }
        }

        Color nodeColor = getNodeColorByType(graph->nodes[i].type);

        Color nodeLeftGradientColor = {
            (unsigned char)Clamp((int)nodeColor.r + 40 + glareOffset, 0, 255),
            (unsigned char)Clamp((int)nodeColor.g + 40 + glareOffset, 0, 255),
            (unsigned char)Clamp((int)nodeColor.b + 40 + glareOffset, 0, 255),
            nodeColor.a};

        Color nodeRightGradientColor = {
            (unsigned char)Clamp((int)nodeColor.r - 60 + glareOffset, 0, 255),
            (unsigned char)Clamp((int)nodeColor.g - 60 + glareOffset, 0, 255),
            (unsigned char)Clamp((int)nodeColor.b - 60 + glareOffset, 0, 255),
            nodeColor.a};

        float fullRadius = roundness * fminf(width, height) / 2.0f;

        Color nodeBackgroundColor = {
            (unsigned char)Clamp((int)glareOffset + 5, 0, 255),
            (unsigned char)Clamp((int)glareOffset + 5, 0, 255),
            (unsigned char)Clamp((int)glareOffset + 5, 0, 255),
            120};

        DrawRectangleRounded(
            (Rectangle){x, y, width, height},
            roundness, segments, nodeBackgroundColor);

        if (cgEd->isLowSpecModeOn)
        {
            DrawCircleSector(
                (Vector2){x + fullRadius - 2, y + fullRadius - 2},
                fullRadius, 180, 270, segments, nodeColor);

            DrawCircleSector(
                (Vector2){x + width - fullRadius + 2, y + fullRadius - 2},
                fullRadius, 270, 360, segments, nodeColor);

            DrawRectangle(x + fullRadius - 2, y - 2, width - 2 * fullRadius + 4, fullRadius, nodeColor);

            DrawRectangle(x - 2, y + fullRadius - 2, width + 4, 38 - fullRadius, nodeColor);
        }
        else
        {
            DrawCircleSector(
                (Vector2){x + fullRadius - 2, y + fullRadius - 2},
                fullRadius, 180, 270, segments, nodeLeftGradientColor);

            DrawCircleSector(
                (Vector2){x + width - fullRadius + 2, y + fullRadius - 2},
                fullRadius, 270, 360, segments, nodeRightGradientColor);

            DrawRectangleGradientH(x + fullRadius - 2, y - 2, width - 2 * fullRadius + 4, fullRadius, nodeLeftGradientColor, nodeRightGradientColor);

            DrawRectangleGradientH(x - 2, y + fullRadius - 2, width + 4, 38 - fullRadius, nodeLeftGradientColor, nodeRightGradientColor);
        }

        DrawRectangleRoundedLinesEx((Rectangle){x - 1, y - 1, width + 2, height + 2}, roundness, segments, 2.0f + 1.0f / cgEd->zoom, WHITE);

        DrawTextEx(cgEd->font, NodeTypeToString(graph->nodes[i].type),
                   (Vector2){x + 8, y + 6}, 28, 1, WHITE);

        if (getIsEditableByType(graph->nodes[i].type))
        {
            Rectangle gearRect = {graph->nodes[i].position.x + getNodeInfoByType(graph->nodes[i].type, WIDTH) - 18 - fullRadius / 5, graph->nodes[i].position.y + 5 + fullRadius / 5, 16, 16};

            Rectangle src = {0, 0, cgEd->gearTxt.width, cgEd->gearTxt.height};
            Rectangle dst = {gearRect.x, gearRect.y, 15, 15};
            Vector2 origin = {0, 0};
            DrawTexturePro(cgEd->gearTxt, src, dst, origin, 0.0f, WHITE);

            if (CheckCollisionPointRec(cgEd->mousePos, gearRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                cgEd->editingNodeNameIndex = i;
                textBoxRect = (Rectangle){graph->nodes[i].position.x + getNodeInfoByType(graph->nodes[i].type, WIDTH) + 10, graph->nodes[i].position.y, MeasureTextEx(cgEd->font, graph->nodes[i].name, 16, 2).x + 25, 30};
            }
            else if (cgEd->editingNodeNameIndex == i)
            {
                HandleVarNameTextBox(cgEd, textBoxRect, graph->nodes[cgEd->editingNodeNameIndex].name, cgEd->editingNodeNameIndex, graph);
                cgEd->delayFrames = true;

                if (CheckCollisionPointRec(cgEd->mousePos, textBoxRect))
                    cgEd->isDraggingScreen = false;

                if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(cgEd->mousePos, textBoxRect)))
                {
                    cgEd->editingNodeNameIndex = -1;
                    cgEd->engineDelayFrames = true;
                }
            }
        }
    }

    int hoveredPinIndex = -1;

    for (int i = 0; i < graph->pinCount; i++)
    {
        int currNodeIndex = -1;

        for (int j = 0; j < graph->nodeCount; j++)
        {
            if (graph->nodes[j].id == graph->pins[i].nodeID)
            {
                currNodeIndex = j;
                break;
            }
        }

        if (currNodeIndex == -1)
        {
            TraceLog(LOG_WARNING, "Pin %d has no matching node (ID %d)", i, graph->pins[i].nodeID);
            continue;
        }

        Vector2 nodePos = graph->nodes[currNodeIndex].position;
        int xOffset = graph->pins[i].isInput ? 5 : (getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH) - 20);
        int yOffset = 52 + graph->pins[i].posInNode * 30;

        graph->pins[i].position = (Vector2){nodePos.x + xOffset + 5, nodePos.y + yOffset};

        if (graph->pins[i].type == PIN_NONE)
        {
            continue;
        }
        else if (graph->pins[i].type == PIN_FLOW)
        {
            DrawTriangle((Vector2){nodePos.x + xOffset, nodePos.y + yOffset - 8}, (Vector2){nodePos.x + xOffset, nodePos.y + yOffset + 8}, (Vector2){nodePos.x + xOffset + 15, nodePos.y + yOffset}, WHITE);
            if (CheckCollisionPointRec(cgEd->mousePos, (Rectangle){nodePos.x + xOffset - 5, nodePos.y + yOffset - 15, 25, 31}))
            {
                if (graph->pins[i].isInput)
                {
                    DrawTextEx(cgEd->font, getNodeInputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], (Vector2){(2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 - MeasureTextEx(cgEd->font, getNodeInputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2, nodePos.y + yOffset - 8}, 18, 0, WHITE);
                    DrawLine(graph->pins[i].position.x, graph->pins[i].position.y, (2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 - MeasureTextEx(cgEd->font, getNodeInputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2 - 5, graph->pins[i].position.y, WHITE);
                }
                else
                {
                    DrawTextEx(cgEd->font, getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], (Vector2){(2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 - MeasureTextEx(cgEd->font, getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2 - 5, nodePos.y + yOffset - 8}, 18, 0, WHITE);
                    DrawLine(graph->pins[i].position.x, graph->pins[i].position.y, (2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 + MeasureTextEx(cgEd->font, getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2, graph->pins[i].position.y, WHITE);
                }
                DrawTriangle((Vector2){nodePos.x + xOffset - 2, nodePos.y + yOffset - 10}, (Vector2){nodePos.x + xOffset - 2, nodePos.y + yOffset + 10}, (Vector2){nodePos.x + xOffset + 17, nodePos.y + yOffset}, WHITE);
                hoveredPinIndex = i;
            }
        }
        else if (graph->pins[i].type == PIN_DROPDOWN_COMPARISON_OPERATOR || graph->pins[i].type == PIN_DROPDOWN_GATE || graph->pins[i].type == PIN_DROPDOWN_ARITHMETIC || graph->pins[i].type == PIN_DROPDOWN_KEY_ACTION || graph->pins[i].type == PIN_DROPDOWN_LAYER || graph->pins[i].type == PIN_VARIABLE || graph->pins[i].type == PIN_SPRITE_VARIABLE)
        {
            HandleDropdownMenu(graph, i, hoveredNodeIndex, currNodeIndex, cgEd);
        }
        else if (graph->pins[i].type == PIN_FIELD_NUM || graph->pins[i].type == PIN_FIELD_STRING || graph->pins[i].type == PIN_FIELD_BOOL || graph->pins[i].type == PIN_FIELD_COLOR)
        {
            HandleLiteralNodeField(cgEd, graph, i);
        }
        else if (graph->pins[i].type == PIN_FIELD_KEY)
        {
            HandleKeyNodeField(cgEd, graph, i);
        }
        else if (graph->pins[i].type == PIN_EDIT_HITBOX)
        {
            DrawRectangleRounded((Rectangle){graph->pins[i].position.x - 6, graph->pins[i].position.y - 10, 96, 24}, 0.4f, 4, DARKPURPLE);
            if (CheckCollisionPointRec(cgEd->mousePos, (Rectangle){graph->pins[i].position.x - 6, graph->pins[i].position.y - 10, 96, 24}))
            {
                DrawRectangleRounded((Rectangle){graph->pins[i].position.x - 6, graph->pins[i].position.y - 10, 96, 24}, 0.4f, 4, COLOR_CGED_EDIT_HITBOX_BTN_HOVER);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    // Find texture file name from linked literal node(shouldn't only work with literal nodes)
                    for (int b = 0; b < graph->nodeCount; b++)
                    {
                        if (graph->nodes[b].id == graph->pins[i].nodeID)
                        {
                            for (int c = 0; c < graph->pinCount; c++)
                            {
                                if (graph->nodes[b].inputPins[1] == graph->pins[c].id)
                                {
                                    for (int k = 0; k < graph->linkCount; k++)
                                    {
                                        if (graph->links[k].inputPinID == graph->pins[c].id)
                                        {
                                            for (int d = 0; d < graph->nodeCount; d++)
                                            {
                                                if (graph->nodes[d].outputPins[0] == graph->links[k].outputPinID)
                                                {
                                                    for (int e = 0; e < graph->pinCount; e++)
                                                    {
                                                        if (graph->pins[e].id == graph->nodes[d].inputPins[0])
                                                        {
                                                            cgEd->shouldOpenHitboxEditor = true;
                                                            strmac(cgEd->hitboxEditorFileName, MAX_FILE_NAME, "%s", graph->pins[e].textFieldValue);
                                                            cgEd->hitboxEditingPinID = graph->pins[i].id;
                                                            return;
                                                        }
                                                    }
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            DrawTextEx(cgEd->font, "Edit Hitbox", (Vector2){graph->pins[i].position.x - 2, graph->pins[i].position.y - 6}, 18, 0.2f, WHITE);
        }
        else
        {
            DrawCircle(nodePos.x + xOffset + 5, nodePos.y + yOffset, 5, WHITE);
            if (CheckCollisionPointCircle(cgEd->mousePos, (Vector2){nodePos.x + xOffset + 5, nodePos.y + yOffset}, 12))
            {
                if (graph->pins[i].isInput)
                {
                    DrawTextEx(cgEd->font, getNodeInputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], (Vector2){(2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 - MeasureTextEx(cgEd->font, getNodeInputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2, nodePos.y + yOffset - 8}, 18, 0, WHITE);
                    DrawLine(graph->pins[i].position.x, graph->pins[i].position.y, (2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 - MeasureTextEx(cgEd->font, getNodeInputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2 - 5, graph->pins[i].position.y, WHITE);
                }
                else if (graph->nodes[currNodeIndex].type == NODE_COMPARISON || graph->nodes[currNodeIndex].type == NODE_GATE || graph->nodes[currNodeIndex].type == NODE_LITERAL_NUMBER || graph->nodes[currNodeIndex].type == NODE_LITERAL_STRING || graph->nodes[currNodeIndex].type == NODE_LITERAL_BOOL || graph->nodes[currNodeIndex].type == NODE_LITERAL_COLOR)
                {
                    const char *label = getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode];
                    Vector2 textSize = MeasureTextEx(cgEd->font, label, 18, 0);
                    DrawTextEx(cgEd->font, label,
                               (Vector2){
                                   graph->pins[i].position.x - textSize.x - 12,
                                   graph->pins[i].position.y - textSize.y / 2},
                               18, 0, WHITE);
                    DrawLine(graph->pins[i].position.x, graph->pins[i].position.y, graph->pins[i].position.x - 10, graph->pins[i].position.y, WHITE);
                }
                else
                {
                    DrawTextEx(cgEd->font, getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], (Vector2){(2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 - MeasureTextEx(cgEd->font, getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2 - 5, nodePos.y + yOffset - 8}, 18, 0, WHITE);
                    DrawLine(graph->pins[i].position.x, graph->pins[i].position.y, (2 * nodePos.x + getNodeInfoByType(graph->nodes[currNodeIndex].type, WIDTH)) / 2 + MeasureTextEx(cgEd->font, getNodeOutputNamesByType(graph->nodes[currNodeIndex].type)[graph->pins[i].posInNode], 18, 0).x / 2, graph->pins[i].position.y, WHITE);
                }
                DrawCircle(nodePos.x + xOffset + 5, nodePos.y + yOffset, 7, WHITE);
                hoveredPinIndex = i;
            }
        }
    }

    if (hoveredPinIndex != -1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        cgEd->draggingNodeIndex = -1;
        if (cgEd->lastClickedPin.id == -1)
        {
            cgEd->lastClickedPin = graph->pins[hoveredPinIndex];
        }
        else
        {
            CreateLink(graph, cgEd->lastClickedPin, graph->pins[hoveredPinIndex]);
            cgEd->lastClickedPin = INVALID_PIN;
            cgEd->hasChangedInLastFrame = true;
        }
    }
    else if (hoveredPinIndex == -1 && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)))
    {
        cgEd->lastClickedPin = INVALID_PIN;
    }
    else if (hoveredPinIndex != -1 && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        RemoveConnections(graph, graph->pins[hoveredPinIndex].id);
        cgEd->menuOpen = false;
        cgEd->hasChangedInLastFrame = true;
    }

    if (hoveredPinIndex == -1 && hoveredNodeIndex != -1)
    {
        DrawRectangleRounded((Rectangle){graph->nodes[hoveredNodeIndex].position.x - 1, graph->nodes[hoveredNodeIndex].position.y - 1, getNodeInfoByType(graph->nodes[hoveredNodeIndex].type, WIDTH) + 2, getNodeInfoByType(graph->nodes[hoveredNodeIndex].type, HEIGHT) + 2}, 0.2f, 8, COLOR_CGED_NODE_HOVER);
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C))
        {
            cgEd->copiedNode = graph->nodes[hoveredNodeIndex];
            SetClipboardText(TextFormat("CoreGraph node of type %s", NodeTypeToString(graph->nodes[hoveredNodeIndex].type)));
        }
        cgEd->delayFrames = true;
    }

    if (cgEd->lastClickedPin.id != -1)
    {
        cgEd->cursor = MOUSE_CURSOR_CROSSHAIR;
        Vector2 p1 = cgEd->lastClickedPin.isInput ? cgEd->mousePos : cgEd->lastClickedPin.position;
        Vector2 p2 = cgEd->lastClickedPin.isInput ? cgEd->lastClickedPin.position : cgEd->mousePos;

        DrawCurvedWire(p1, p2, 2.0f + 1.0f / cgEd->zoom, cgEd->lastClickedPin.type == PIN_FLOW ? COLOR_CGED_WIRE_FLOW : COLOR_CGED_WIRE_NEON_BLUE, cgEd->isLowSpecModeOn);
    }

    if (nodeToDelete != -1 && hoveredPinIndex == -1)
    {
        DeleteNode(graph, nodeToDelete);
        cgEd->menuOpen = false;
        cgEd->hasChangedInLastFrame = true;
        return;
    }
}

bool CheckNodeCollisions(CGEditorContext *cgEd, GraphContext *graph)
{
    for (int i = 0; i < graph->nodeCount; i++)
    {
        if (CheckCollisionPointRec(cgEd->mousePos, (Rectangle){graph->nodes[i].position.x, graph->nodes[i].position.y, getNodeInfoByType(graph->nodes[i].type, WIDTH), getNodeInfoByType(graph->nodes[i].type, HEIGHT)}))
        {
            return true;
        }
    }

    return false;
}

const char *Search(const char *haystack, const char *needle)
{
    if (!*needle)
        return haystack;
    for (; *haystack; haystack++)
    {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle))
        {
            const char *h = haystack + 1;
            const char *n = needle + 1;
            while (*n && tolower((unsigned char)*h) == tolower((unsigned char)*n))
            {
                h++;
                n++;
            }
            if (!*n)
                return haystack;
        }
    }
    return NULL;
}

const char *DrawNodeMenu(CGEditorContext *cgEd, RenderTexture2D view)
{
    Color MenuColor = {50, 50, 50, 255};
    Color HighlightColor = {80, 80, 80, 255};

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        cgEd->createNodeMenuFirstFrame = true;
    }

    float searchBarHeight = 30.0f;
    float menuHeight = MENU_ITEM_HEIGHT * MENU_VISIBLE_ITEMS + searchBarHeight + 10;

    int len = strlen(cgEd->nodeMenuSearch);

    int key = GetCharPressed();
    while (key > 0 && len < MAX_SEARCH_BAR_FIELD_SIZE - 1)
    {
        if (key >= 32 && key <= 125)
        {
            cgEd->nodeMenuSearch[len] = (char)key;
            cgEd->nodeMenuSearch[len + 1] = '\0';
            len++;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        if (len > 0)
        {
            cgEd->nodeMenuSearch[len - 1] = '\0';
            len--;
        }
    }

    int filteredItemCategory[subMenuItemCount * menuItemCount];
    int filteredItem[subMenuItemCount * menuItemCount];
    int filteredCount = 0;

    for (int i = 0; i < menuItemCount; i++)
    {
        for (int j = 0; j < subMenuCounts[i]; j++)
        {
            if (strlen(cgEd->nodeMenuSearch) == 0 || (subMenuItems[i][j][0] != '\0' && Search(subMenuItems[i][j], cgEd->nodeMenuSearch)))
            {
                if (filteredCount < subMenuItemCount * menuItemCount)
                {
                    filteredItemCategory[filteredCount] = i;
                    filteredItem[filteredCount] = j;
                    filteredCount++;
                }
            }
        }
    }

    if (cgEd->nodeMenuSearch[0] == '\0')
    {
        filteredCount = menuItemCount;
    }

    int wheelMove = GetMouseWheelMove();

    if (wheelMove < 0 && cgEd->scrollIndexNodeMenu < filteredCount - MENU_VISIBLE_ITEMS)
    {
        cgEd->scrollIndexNodeMenu++;
    }
    else if (wheelMove > 0 && cgEd->scrollIndexNodeMenu > 0)
    {
        cgEd->scrollIndexNodeMenu--;
    }

    if (cgEd->createNodeMenuFirstFrame)
    {
        cgEd->menuPosition = cgEd->rightClickPos;
        if (cgEd->menuPosition.y + menuHeight > cgEd->viewportBoundary.y + cgEd->viewportBoundary.height)
        {
            cgEd->menuPosition.y = cgEd->viewportBoundary.y + cgEd->viewportBoundary.height - menuHeight;
        }
        if (cgEd->menuPosition.x + MENU_WIDTH + SUBMENU_WIDTH > cgEd->viewportBoundary.x + cgEd->viewportBoundary.width)
        {
            cgEd->menuPosition.x = cgEd->viewportBoundary.x + cgEd->viewportBoundary.width - MENU_WIDTH - SUBMENU_WIDTH;
        }

        Rectangle menuRect = {cgEd->menuPosition.x, cgEd->menuPosition.y + searchBarHeight + 10, MENU_WIDTH, menuHeight - searchBarHeight - 10};
        cgEd->submenuPosition.x = (cgEd->menuPosition.x + MENU_WIDTH + SUBMENU_WIDTH > cgEd->screenWidth)
                                      ? (cgEd->menuPosition.x - SUBMENU_WIDTH)
                                      : (cgEd->menuPosition.x + MENU_WIDTH - 15);
        cgEd->submenuPosition.y = cgEd->menuPosition.y + searchBarHeight + 7;
        cgEd->hoveredItem = 0;
        cgEd->createNodeMenuFirstFrame = false;
    }

    DrawRectangleRounded((Rectangle){cgEd->menuPosition.x, cgEd->menuPosition.y, MENU_WIDTH, menuHeight}, 0.1f, 8, MenuColor);
    DrawRectangleRoundedLinesEx((Rectangle){cgEd->menuPosition.x, cgEd->menuPosition.y, MENU_WIDTH, menuHeight}, 0.1f, 8, MENU_BORDER_THICKNESS, WHITE);

    if (cgEd->nodeMenuSearch[0] != '\0')
    {
        for (int i = 0; i < (int)MENU_VISIBLE_ITEMS; i++)
        {
            int listIndex = i + cgEd->scrollIndexNodeMenu;
            if (listIndex >= filteredCount)
                break;

            const char *pickedItemName = subMenuItems[filteredItemCategory[listIndex]][filteredItem[listIndex]];
            Rectangle itemRect = {cgEd->menuPosition.x, cgEd->menuPosition.y + searchBarHeight + 10 + i * MENU_ITEM_HEIGHT, MENU_WIDTH, MENU_ITEM_HEIGHT};

            if (CheckCollisionPointRec(cgEd->mousePos, itemRect))
            {
                cgEd->cursor = MOUSE_CURSOR_POINTING_HAND;
                DrawRectangleRec(itemRect, HighlightColor);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    cgEd->delayFrames = true;
                    cgEd->hasChangedInLastFrame = true;
                    cgEd->menuOpen = false;
                    cgEd->nodeMenuSearch[0] = '\0';
                    return pickedItemName;
                }
            }
            DrawTextEx(cgEd->font, pickedItemName, (Vector2){itemRect.x + 20, itemRect.y + 12}, 25, 1, WHITE);
            DrawLine(itemRect.x, itemRect.y + MENU_ITEM_HEIGHT - 1, itemRect.x + MENU_WIDTH, itemRect.y + MENU_ITEM_HEIGHT - 1, DARKGRAY);
        }
    }
    else
    {
        for (int i = 0; i < (int)MENU_VISIBLE_ITEMS; i++)
        {
            int listIndex = i + cgEd->scrollIndexNodeMenu;
            if (listIndex >= menuItemCount)
            {
                break;
            }

            Rectangle itemRect = {cgEd->menuPosition.x, cgEd->menuPosition.y + searchBarHeight + 10 + i * MENU_ITEM_HEIGHT, MENU_WIDTH, MENU_ITEM_HEIGHT};

            if (CheckCollisionPointRec(cgEd->mousePos, itemRect))
            {
                cgEd->hoveredItem = listIndex;
                cgEd->submenuPosition.x = (cgEd->menuPosition.x + MENU_WIDTH + SUBMENU_WIDTH > cgEd->screenWidth)
                                              ? (cgEd->menuPosition.x - SUBMENU_WIDTH)
                                              : (cgEd->menuPosition.x + MENU_WIDTH - 15);
                cgEd->submenuPosition.y = itemRect.y - 3;
            }

            if (listIndex == cgEd->hoveredItem)
                DrawRectangleRec(itemRect, HighlightColor);

            DrawTextEx(cgEd->font, menuItems[listIndex], (Vector2){itemRect.x + 20, itemRect.y + 12}, 25, 1, WHITE);
            DrawLine(itemRect.x, itemRect.y + MENU_ITEM_HEIGHT - 1, itemRect.x + MENU_WIDTH, itemRect.y + MENU_ITEM_HEIGHT - 1, DARKGRAY);
        }
    }

    int maxScroll = (filteredCount > 0 ? filteredCount : menuItemCount) - MENU_VISIBLE_ITEMS;
    if (maxScroll < 1)
    {
        maxScroll = 1;
    }

    float scrollTrackHeight = MENU_ITEM_HEIGHT * MENU_VISIBLE_ITEMS - 16;

    float scrollBarHeight = scrollTrackHeight * ((float)MENU_VISIBLE_ITEMS / ((filteredCount > 5) ? filteredCount : 1));
    if (scrollBarHeight < 20.0f)
    {
        scrollBarHeight = 20.0f;
    }
    if (scrollBarHeight > scrollTrackHeight)
    {
        scrollBarHeight = scrollTrackHeight;
    }

    DrawRectangleRounded((Rectangle){cgEd->menuPosition.x + 2, cgEd->menuPosition.y + searchBarHeight + cgEd->scrollIndexNodeMenu * ((scrollTrackHeight - scrollBarHeight) / (maxScroll + 1)) + 16, 8, scrollBarHeight}, 0.8f, 4, GRAY_150);

    if (cgEd->nodeMenuSearch[0] == '\0' && cgEd->hoveredItem >= 0 && cgEd->hoveredItem < menuItemCount)
    {
        DrawRectangleRounded((Rectangle){cgEd->submenuPosition.x, cgEd->submenuPosition.y, SUBMENU_WIDTH, subMenuCounts[cgEd->hoveredItem] * MENU_ITEM_HEIGHT}, 0.1f, 2, MenuColor);
        DrawRectangleRoundedLinesEx((Rectangle){cgEd->submenuPosition.x, cgEd->submenuPosition.y, SUBMENU_WIDTH, subMenuCounts[cgEd->hoveredItem] * MENU_ITEM_HEIGHT}, 0.1f, 2, MENU_BORDER_THICKNESS, WHITE);
        DrawRectangleGradientH(cgEd->submenuPosition.x - 5, cgEd->submenuPosition.y + 3, 20, MENU_ITEM_HEIGHT, HighlightColor, MenuColor);
        for (int j = 0; j < subMenuCounts[cgEd->hoveredItem]; j++)
        {
            Rectangle subItemRect = {cgEd->submenuPosition.x, cgEd->submenuPosition.y + j * MENU_ITEM_HEIGHT, SUBMENU_WIDTH, MENU_ITEM_HEIGHT};
            if (CheckCollisionPointRec(cgEd->mousePos, subItemRect))
            {
                cgEd->cursor = MOUSE_CURSOR_POINTING_HAND;
                DrawRectangleRounded(subItemRect, 0.2f, 2, HighlightColor);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    cgEd->delayFrames = true;
                    cgEd->hasChangedInLastFrame = true;
                    cgEd->menuOpen = false;
                    return subMenuItems[cgEd->hoveredItem][j];
                }
            }
            DrawTextEx(cgEd->font, subMenuItems[cgEd->hoveredItem][j], (Vector2){cgEd->submenuPosition.x + 20, cgEd->submenuPosition.y + j * MENU_ITEM_HEIGHT + 10}, 25, 0, WHITE);
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        cgEd->nodeMenuSearch[0] = '\0';
        cgEd->menuOpen = false;
    }

    DrawRectangle(cgEd->menuPosition.x + 5, cgEd->menuPosition.y + 5, MENU_WIDTH - 10, searchBarHeight, DARKGRAY);

    DrawTextEx(cgEd->font, cgEd->nodeMenuSearch[0] == '\0' ? "Type to search" : cgEd->nodeMenuSearch, (Vector2){cgEd->menuPosition.x + 10, cgEd->menuPosition.y + 10}, 20, 0, WHITE);

    return NULL;
}

void HandleDragging(CGEditorContext *cgEd, GraphContext *graph)
{
    static Vector2 dragOffset;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && cgEd->draggingNodeIndex == -1 && cgEd->nodeFieldPinFocused == -1)
    {
        cgEd->fps = 140;
        for (int i = 0; i < graph->nodeCount; i++)
        {
            if (CheckCollisionPointRec(cgEd->mousePos, (Rectangle){graph->nodes[i].position.x, graph->nodes[i].position.y, getNodeInfoByType(graph->nodes[i].type, WIDTH), getNodeInfoByType(graph->nodes[i].type, HEIGHT)}))
            {
                cgEd->draggingNodeIndex = i;
                dragOffset = (Vector2){cgEd->mousePos.x - graph->nodes[i].position.x, cgEd->mousePos.y - graph->nodes[i].position.y};
                return;
            }
        }

        cgEd->isDraggingScreen = true;
        return;
    }
    else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && cgEd->draggingNodeIndex != -1 && cgEd->nodeFieldPinFocused == -1)
    {
        cgEd->cursor = MOUSE_CURSOR_RESIZE_ALL;
        float newX = cgEd->mousePos.x - dragOffset.x;
        float newY = cgEd->mousePos.y - dragOffset.y;
        if (newX != graph->nodes[cgEd->draggingNodeIndex].position.x || newY != graph->nodes[cgEd->draggingNodeIndex].position.y)
        {
            cgEd->hasChangedInLastFrame = true;
        }
        graph->nodes[cgEd->draggingNodeIndex].position.x = newX;
        graph->nodes[cgEd->draggingNodeIndex].position.y = newY;
        DrawRectangleRounded((Rectangle){graph->nodes[cgEd->draggingNodeIndex].position.x, graph->nodes[cgEd->draggingNodeIndex].position.y, getNodeInfoByType(graph->nodes[cgEd->draggingNodeIndex].type, WIDTH), getNodeInfoByType(graph->nodes[cgEd->draggingNodeIndex].type, HEIGHT)}, 0.2f, 8, COLOR_CGED_NODE_HOVER);
    }
    else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && cgEd->isDraggingScreen)
    {
        Vector2 delta = Vector2Scale(GetMouseDelta(), 1.0f / cgEd->zoom);
        cgEd->cameraOffset.x += Vector2Scale(GetMouseDelta(), 1.0f / cgEd->zoom).x;
        cgEd->cameraOffset.y += Vector2Scale(GetMouseDelta(), 1.0f / cgEd->zoom).y;
        for (int i = 0; i < graph->nodeCount; i++)
        {

            graph->nodes[i].position.x += delta.x;
            graph->nodes[i].position.y += delta.y;
        }
    }
    else if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
    {
        cgEd->fps = 60;
        cgEd->draggingNodeIndex = -1;
        cgEd->isDraggingScreen = false;
    }
}

void DrawFullTexture(CGEditorContext *cgEd, GraphContext *graph, RenderTexture2D view, RenderTexture2D dot)
{
    BeginTextureMode(view);
    ClearBackground(COLOR_CGED_BACKGROUND);

    HandleDragging(cgEd, graph);

    DrawBackgroundGrid(cgEd, 40, dot);

    DrawNodes(cgEd, graph);

    if (cgEd->menuOpen)
    {
        const char *createdNode = DrawNodeMenu(cgEd, view);

        if (createdNode != NULL)
        {
            NodeType newNodeType = StringToNodeType(createdNode);
            if (!CreateNode(graph, newNodeType, cgEd->rightClickPos))
            {
                cgEd->hasFatalErrorOccurred = true;
                AddToLogFromEditor(cgEd, "Error creating node{C230}", LOG_LEVEL_ERROR);
                return;
            }
            else if (newNodeType == NODE_CREATE_NUMBER || newNodeType == NODE_CREATE_STRING || newNodeType == NODE_CREATE_BOOL || newNodeType == NODE_CREATE_COLOR || newNodeType == NODE_CREATE_SPRITE)
            {
                graph->variables = realloc(graph->variables, sizeof(char *) * (graph->variablesCount + 1));
                graph->variables[graph->variablesCount] = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "%s", graph->nodes[graph->nodeCount - 1].name);

                graph->variableTypes = realloc(graph->variableTypes, sizeof(int) * (graph->variablesCount + 1));
                graph->variableTypes[graph->variablesCount] = graph->nodes[graph->nodeCount - 1].type;

                graph->variablesCount++;
            }
            cgEd->rightClickPos = (Vector2){0, 0};
        }
    }

    EndTextureMode();
}

bool CheckOpenMenus(CGEditorContext *cgEd)
{
    return cgEd->draggingNodeIndex != -1 || cgEd->lastClickedPin.id != -1 || cgEd->menuOpen || cgEd->nodeDropdownFocused != -1 || cgEd->nodeFieldPinFocused != -1 || cgEd->editingNodeNameIndex != -1;
}

void HandleEditor(CGEditorContext *cgEd, GraphContext *graph, RenderTexture2D *viewport, Vector2 mousePos, bool draggingDisabled, bool isSecondFrame)
{
    cgEd->newLogMessage = false;
    cgEd->cursor = MOUSE_CURSOR_ARROW;

    cgEd->screenWidth = viewport->texture.width;
    cgEd->screenHeight = viewport->texture.height;
    cgEd->mousePos = mousePos;

    static RenderTexture2D dot;

    if (isSecondFrame)
    {
        dot = LoadRenderTexture(15, 15);
        SetTextureFilter(dot.texture, TEXTURE_FILTER_BILINEAR);
        BeginTextureMode(dot);
        ClearBackground(BLANK);
        DrawRectangleRounded((Rectangle){0, 0, 15, 15}, 1.0f, 8, GRAY_128);
        EndTextureMode();

        cgEd->isFirstFrame = false;
    }

    if (draggingDisabled)
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        cgEd->menuOpen = true;
        cgEd->rightClickPos = cgEd->mousePos;
        cgEd->scrollIndexNodeMenu = 0;
        cgEd->delayFrames = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V) && cgEd->copiedNode.id != -1 && cgEd->nodeFieldPinFocused == -1 && cgEd->editingNodeNameIndex == -1)
    {
        if (!DuplicateNode(graph, &cgEd->copiedNode, cgEd->mousePos))
        {
            cgEd->hasFatalErrorOccurred = true;
            AddToLogFromEditor(cgEd, "Error duplicating node{C231}", LOG_LEVEL_ERROR);
            return;
        }
        else if (cgEd->copiedNode.type == NODE_CREATE_NUMBER || cgEd->copiedNode.type == NODE_CREATE_STRING || cgEd->copiedNode.type == NODE_CREATE_BOOL || cgEd->copiedNode.type == NODE_CREATE_COLOR || cgEd->copiedNode.type == NODE_CREATE_SPRITE)
        {
            strmac(graph->nodes[graph->nodeCount - 1].name, MAX_VARIABLE_NAME_SIZE, "%s", AssignAvailableVarName(graph, graph->nodes[graph->nodeCount - 1].name));

            graph->variables = realloc(graph->variables, sizeof(char *) * (graph->variablesCount + 1));
            graph->variables[graph->variablesCount] = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "%s", graph->nodes[graph->nodeCount - 1].name);

            graph->variableTypes = realloc(graph->variableTypes, sizeof(int) * (graph->variablesCount + 1));
            graph->variableTypes[graph->variablesCount] = graph->nodes[graph->nodeCount - 1].type;

            graph->variablesCount++;
        }
        cgEd->delayFrames = true;
        cgEd->engineDelayFrames = true;
    }

    if (CheckNodeCollisions(cgEd, graph) || IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        cgEd->cursor = MOUSE_CURSOR_POINTING_HAND;
        DrawFullTexture(cgEd, graph, *viewport, dot);
        cgEd->delayFrames = true;
    }
    else if (CheckOpenMenus(cgEd))
    {
        DrawFullTexture(cgEd, graph, *viewport, dot);
        cgEd->delayFrames = true;
    }
    else if (cgEd->delayFrames == true)
    {
        DrawFullTexture(cgEd, graph, *viewport, dot);
        cgEd->delayFrames = false;
    }
}