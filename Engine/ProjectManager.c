#include "ProjectManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

void DrawMovingDotAlongRectangle()
{
    static float t = 0.0f;
    float speed = 8.0f;

    int x = 475;
    int y = 180;
    int w = 652;
    int h = 142;

    float perimeter = 2 * (w + h);

    t += speed;
    if (t > perimeter)
        t -= perimeter;

    float dotX = x;
    float dotY = y;

    if (t < w)
    {
        dotX = x + t;
        dotY = y;
    }
    else if (t < w + h)
    {
        dotX = x + w;
        dotY = y + (t - w);
    }
    else if (t < w + h + w)
    {
        dotX = x + w - (t - w - h);
        dotY = y + h;
    }
    else
    {
        dotX = x;
        dotY = y + h - (t - w - h - w);
    }

    DrawCircle((int)dotX, (int)dotY, 5, (Color){180, 100, 200, 255});
}

void DrawX(Vector2 center, float size, float thickness, Color color)
{
    float half = size / 2;

    Vector2 p1 = {center.x - half, center.y - half};
    Vector2 p2 = {center.x + half, center.y + half};
    Vector2 p3 = {center.x - half, center.y + half};
    Vector2 p4 = {center.x + half, center.y - half};

    DrawLineEx(p1, p2, thickness, color);
    DrawLineEx(p3, p4, thickness, color);
}

void DrawTopButtons()
{

    DrawLineEx((Vector2){0, 1}, (Vector2){1600, 1}, 4.0f, WHITE);
    DrawLineEx((Vector2){1, 0}, (Vector2){1, 1000}, 4.0f, WHITE);
    DrawLineEx((Vector2){0, 999}, (Vector2){1600, 999}, 4.0f, WHITE);
    DrawLineEx((Vector2){1599, 0}, (Vector2){1599, 1000}, 4.0f, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){1550, 0, 50, 50}))
    {
        DrawRectangle(1550, 0, 50, 50, RED);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            CloseWindow();
        }
    }

    DrawX((Vector2){1575, 25}, 20, 2, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){1500, 0, 50, 50}))
    {
        DrawRectangle(1500, 0, 50, 50, GRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            MinimizeWindow();
        }
    }

    DrawLineEx((Vector2){1515, 25}, (Vector2){1535, 25}, 2, WHITE);
}

typedef enum
{
    MAIN_WINDOW_BUTTON_NONE,
    MAIN_WINDOW_BUTTON_LOAD,
    MAIN_WINDOW_BUTTON_CREATE
} MainWindowButton;

int MainWindow(Font font, Font fontRE)
{
    Rectangle btnLoad = {0, 0, 798, 1000};
    Rectangle btnCreate = {802, 0, 796, 1000};

    Vector2 mousePos = GetMousePosition();

    static MainWindowButton hoveredButton = MAIN_WINDOW_BUTTON_NONE;

    BeginDrawing();
    ClearBackground((Color){40, 42, 54, 255});

    if (GetMouseDelta().x != 0 || GetMouseDelta().y != 0)
    {
        if (CheckCollisionPointRec(mousePos, (Rectangle){484, 189, 632, 122}))
        {
            hoveredButton = MAIN_WINDOW_BUTTON_NONE;
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        else if (CheckCollisionPointRec(mousePos, btnLoad))
        {
            hoveredButton = MAIN_WINDOW_BUTTON_LOAD;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        else if (CheckCollisionPointRec(mousePos, btnCreate) && !CheckCollisionPointRec(mousePos, (Rectangle){1500, 0, 100, 50}))
        {
            hoveredButton = MAIN_WINDOW_BUTTON_CREATE;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        else
        {
            hoveredButton = MAIN_WINDOW_BUTTON_NONE;
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            if (hoveredButton == MAIN_WINDOW_BUTTON_LOAD)
            {
                hoveredButton = MAIN_WINDOW_BUTTON_NONE;
                return PROJECT_MANAGER_WINDOW_MODE_LOAD;
            }
            else if (hoveredButton == MAIN_WINDOW_BUTTON_CREATE)
            {
                hoveredButton = MAIN_WINDOW_BUTTON_NONE;
                return PROJECT_MANAGER_WINDOW_MODE_CREATE;
            }
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            hoveredButton = MAIN_WINDOW_BUTTON_LOAD;
        }
        else if (IsKeyPressed(KEY_RIGHT))
        {
            hoveredButton = MAIN_WINDOW_BUTTON_CREATE;
        }
    }

    if (hoveredButton == MAIN_WINDOW_BUTTON_LOAD)
    {
        DrawRectangle(0, 0, 800, 1000, (Color){128, 128, 128, 20});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoveredButton == MAIN_WINDOW_BUTTON_LOAD)
        {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            return PROJECT_MANAGER_WINDOW_MODE_LOAD;
        }
    }
    else if (hoveredButton == MAIN_WINDOW_BUTTON_CREATE)
    {
        DrawRectangle(800, 0, 1600, 1000, (Color){128, 128, 128, 20});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoveredButton == MAIN_WINDOW_BUTTON_CREATE && !CheckCollisionPointRec(mousePos, (Rectangle){1500, 0, 100, 50}))
        {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            return PROJECT_MANAGER_WINDOW_MODE_CREATE;
        }
    }

    DrawTopButtons();

    DrawLineEx((Vector2){800, 0}, (Vector2){800, 1000}, 4.0f, WHITE);

    DrawRectangleRounded((Rectangle){482, 187, 636, 126}, 0.6f, 16, WHITE);
    DrawRectangleRounded((Rectangle){485, 190, 630, 120}, 0.6f, 16, (Color){202, 97, 255, 255});

    DrawTextEx(fontRE, "R", (Vector2){500, 180}, 130, 0, (Color){255, 255, 255, 255});
    DrawTextEx(font, "apid Engine", (Vector2){605, 200}, 100, 0, (Color){255, 255, 255, 255});

    DrawTextEx(font, "Load", (Vector2){290, 540}, 80, 0, WHITE);
    DrawTextEx(font, "project", (Vector2){260, 630}, 80, 0, WHITE);

    DrawTextEx(font, "Create", (Vector2){1080, 540}, 80, 0, WHITE);
    DrawTextEx(font, "project", (Vector2){1075, 630}, 80, 0, WHITE);

    DrawMovingDotAlongRectangle();

    return PROJECT_MANAGER_WINDOW_MODE_MAIN;
}

int WindowLoadProject(char *projectFileName, Font font)
{
    Rectangle backButton = {1, 0, 65, 1600};

    Vector2 mousePos = GetMousePosition();

    FilePathList files = LoadDirectoryFiles(TextFormat("..%cProjects", PATH_SEPARATOR));

    int yPosition = 80;

    static int selectedProject = 0;
    static bool showSelectorArrow = true;

    if (IsKeyPressed(KEY_LEFT))
    {
        return PROJECT_MANAGER_WINDOW_MODE_MAIN;
    }

    BeginDrawing();
    ClearBackground((Color){40, 42, 54, 255});

    DrawTopButtons();

    DrawRectangleRec(backButton, CLITERAL(Color){70, 70, 70, 150});

    if (CheckCollisionPointRec(mousePos, backButton))
    {
        DrawRectangleRec(backButton, CLITERAL(Color){255, 255, 255, 50});
        DrawTextEx(font, "<", (Vector2){10, 490}, 70, 0, WHITE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return PROJECT_MANAGER_WINDOW_MODE_MAIN;
        }
    }
    else
    {
        DrawTextEx(font, "<", (Vector2){15, 500}, 50, 0, WHITE);
    }

    if (CheckCollisionPointRec(mousePos, backButton))
    {
        DrawRectangleRec(backButton, CLITERAL(Color){255, 255, 255, 100});

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return PROJECT_MANAGER_WINDOW_MODE_MAIN;
        }
    }

    static float blinkTimer = 0;
    blinkTimer += GetFrameTime();

    for (int i = 0; i < files.count; i++)
    {
        const char *fileName = GetFileName(files.paths[i]);
        int fileNameLength = MeasureTextEx(font, fileName, 35, 1).x;
        DrawTextEx(font, fileName, (Vector2){(GetScreenWidth() - fileNameLength) / 2, yPosition}, 35, 1, WHITE);
        if (CheckCollisionPointRec(mousePos, (Rectangle){(GetScreenWidth() - fileNameLength) / 2 - 5, yPosition - 5, fileNameLength + 10, 45}))
        {
            if(GetMouseDelta().x != 0 || GetMouseDelta().y != 0){
                selectedProject = i;
            }
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                strcpy(projectFileName, fileName);
                return PROJECT_MANAGER_WINDOW_MODE_EXIT;
            }
        }
        yPosition += 50;
    }

    if (blinkTimer >= 0.3f)
    {
        blinkTimer = 0;
        showSelectorArrow = !showSelectorArrow;
    }

    if (IsKeyPressed(KEY_DOWN))
    {
        if (selectedProject == files.count - 1)
        {
            selectedProject = 0;
        }
        else
        {
            selectedProject++;
        }
        showSelectorArrow = true;
        blinkTimer = 0;
    }
    if (IsKeyPressed(KEY_UP))
    {
        if (selectedProject == 0)
        {
            selectedProject = files.count - 1;
        }
        else
        {
            selectedProject--;
        }
        showSelectorArrow = true;
        blinkTimer = 0;
    }

    if (showSelectorArrow)
    {
        int fileNameLength = MeasureTextEx(font, GetFileName(files.paths[selectedProject]), 35, 1).x;
        DrawTextEx(font, ">", (Vector2){(GetScreenWidth() - fileNameLength) / 2 - 30, 80 + selectedProject * 50}, 35, 0, (Color){202, 97, 255, 255});
        DrawTextEx(font, "<", (Vector2){(GetScreenWidth() + fileNameLength) / 2 + 10, 80 + selectedProject * 50}, 35, 0, (Color){202, 97, 255, 255});
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        strcpy(projectFileName, GetFileName(files.paths[selectedProject]));
        return PROJECT_MANAGER_WINDOW_MODE_EXIT;
    }

    UnloadDirectoryFiles(files);

    return PROJECT_MANAGER_WINDOW_MODE_LOAD;
}

bool CreateProject(ProjectOptions PO)
{
    char projectPath[512];

    sprintf(projectPath, "..%cProjects%c%s", PATH_SEPARATOR, PATH_SEPARATOR, PO.projectName);

    if (MAKE_DIR(projectPath) != 0)
    {
        return false;
    }

    char mainPath[512];

    sprintf(mainPath, "%s%c%s.c", projectPath, PATH_SEPARATOR, PO.projectName);

    FILE *file = fopen(mainPath, "w");

    if (file == NULL)
    {
        return false;
    }

    fclose(file);

    sprintf(mainPath, "%s%c%s.cg", projectPath, PATH_SEPARATOR, PO.projectName);

    file = fopen(mainPath, "w");

    if (file == NULL)
    {
        return false;
    }

    fclose(file);

    char foldersPath[512];

    sprintf(foldersPath, "%s%cAssets", projectPath, PATH_SEPARATOR);

    if (MAKE_DIR(foldersPath) != 0)
    {
        return false;
    }

    return true;
}

int WindowCreateProject(char *projectFileName, Font font)
{
    Rectangle backButton = {1, 0, 65, 1600};
    static Rectangle textBox = {700, 230, 250, 40};
    static char inputText[MAX_FILE_NAME] = "";
    static int letterCount = 0;
    static bool isFocused = true;

    static ProjectOptions PO;

    Vector2 mousePos = GetMousePosition();

    if (IsKeyPressed(KEY_LEFT))
    {
        return PROJECT_MANAGER_WINDOW_MODE_MAIN;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isFocused = CheckCollisionPointRec(mousePos, textBox);
    }

    if (isFocused)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && (letterCount < MAX_FILE_NAME))
            {
                inputText[letterCount] = (char)key;
                letterCount++;
                inputText[letterCount] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0)
        {
            letterCount--;
            inputText[letterCount] = '\0';
        }
    }

    BeginDrawing();
    ClearBackground((Color){40, 42, 54, 255});

    DrawTopButtons();

    DrawRectangleRec(backButton, CLITERAL(Color){70, 70, 70, 150});

    if (CheckCollisionPointRec(mousePos, backButton))
    {
        DrawRectangleRec(backButton, CLITERAL(Color){255, 255, 255, 50});
        DrawTextEx(font, "<", (Vector2){10, 490}, 70, 0, WHITE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return PROJECT_MANAGER_WINDOW_MODE_MAIN;
        }
    }
    else
    {
        DrawTextEx(font, "<", (Vector2){15, 500}, 50, 0, WHITE);
    }

    DrawRectangleRounded(textBox, 2.0f, 8, LIGHTGRAY);
    DrawRectangleRoundedLinesEx(textBox, 2.0f, 8, 2, isFocused ? WHITE : DARKGRAY);
    const char *subStr;
    if (MeasureTextEx(font, inputText, 30, 0).x > textBox.width)
    {
        int len = strlen(inputText);
        for (int i = 0; i < len; i++)
        {
            subStr = &inputText[i];
            if (MeasureTextEx(font, subStr, 30, 0).x <= textBox.width - MeasureText("_", 30) - 15)
            {
                DrawTextEx(font, subStr, (Vector2){textBox.x + 5, textBox.y + 10}, 30, 0, BLACK);
                break;
            }
        }
    }
    else
    {
        DrawTextEx(font, inputText, (Vector2){textBox.x + 5, textBox.y + 10}, 30, 0, BLACK);
    }

    if (isFocused && MeasureTextEx(font, inputText, 30, 0).x < textBox.width && (GetTime() - (int)GetTime() < 0.5))
    {
        DrawText("_", textBox.x + 10 + MeasureTextEx(font, inputText, 30, 0).x, textBox.y + 10, 30, BLACK);
    }
    else if (isFocused && MeasureTextEx(font, inputText, 30, 0).x > textBox.width && (GetTime() - (int)GetTime() < 0.5))
    {
        DrawText("_", textBox.x + 10 + MeasureTextEx(font, subStr, 30, 0).x, textBox.y + 10, 30, BLACK);
    }

    DrawTextEx(font, "Enter project name:", (Vector2){textBox.x + 10, textBox.y - 30}, 25, 0, WHITE);

    DrawTextEx(font, "2D", (Vector2){700, 330}, 30, 0, WHITE);
    DrawRectangle(750, 330, 30, 30, WHITE);
    DrawRectangleLinesEx((Rectangle){750, 330, 30, 30}, 3, BLACK);
    if (!PO.is3D && CheckCollisionPointRec(mousePos, (Rectangle){750, 330, 30, 30}))
    {
        DrawX((Vector2){765, 345}, 20, 5, (Color){202, 97, 255, 255});
    }
    else if (!PO.is3D)
    {
        DrawX((Vector2){765, 345}, 15, 3, (Color){202, 97, 255, 255});
    }
    if (CheckCollisionPointRec(mousePos, (Rectangle){750, 330, 30, 30}))
    {
        DrawRectangle(750, 330, 30, 30, (Color){255, 255, 255, 150});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            PO.is3D = false;
        }
    }

    DrawTextEx(font, "3D", (Vector2){840, 330}, 30, 0, WHITE);
    DrawRectangle(890, 330, 30, 30, WHITE);
    DrawRectangleLinesEx((Rectangle){890, 330, 30, 30}, 3, BLACK);
    if (PO.is3D && CheckCollisionPointRec(mousePos, (Rectangle){890, 330, 30, 30}))
    {
        DrawX((Vector2){905, 345}, 20, 5, (Color){202, 97, 255, 255});
    }
    else if (PO.is3D)
    {
        DrawX((Vector2){905, 345}, 15, 3, (Color){202, 97, 255, 255});
    }
    if (CheckCollisionPointRec(mousePos, (Rectangle){890, 330, 30, 30}))
    {
        DrawRectangle(890, 330, 30, 30, (Color){255, 255, 255, 150});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            PO.is3D = true;
        }
    }

    if (PO.is3D)
    {
        DrawTextEx(font, "3D mode is currently unavailable", (Vector2){690, 475}, 20, 0, RED);
    }

    if (letterCount > 0 && !PO.is3D)
    {
        DrawRectangleRounded((Rectangle){700, 500, 250, 50}, 2.0f, 8, (Color){202, 97, 255, 255});
        DrawRectangleRoundedLinesEx((Rectangle){700, 500, 250, 50}, 2.0f, 8, 3, WHITE);
        DrawTextEx(font, "Create project", (Vector2){730, 507}, 32, 0, WHITE);
        if (CheckCollisionPointRec(mousePos, (Rectangle){700, 500, 250, 50}))
        {
            DrawRectangleRounded((Rectangle){700, 500, 250, 50}, 2.0f, 8, (Color){255, 255, 255, 150});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                strcpy(PO.projectName, inputText);
                if (!CreateProject(PO))
                {
                    exit(1);
                }
                strcpy(projectFileName, inputText);
                return PROJECT_MANAGER_WINDOW_MODE_EXIT;
            }
        }
    }
    else
    {
        DrawRectangleRounded((Rectangle){700, 500, 250, 50}, 2.0f, 8, DARKGRAY);
        DrawRectangleRoundedLinesEx((Rectangle){700, 500, 250, 50}, 2.0f, 8, 3, BLACK);
        DrawTextEx(font, "Create project", (Vector2){730, 507}, 32, 0, LIGHTGRAY);
        if (CheckCollisionPointRec(mousePos, (Rectangle){700, 500, 250, 50}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !PO.is3D)
        {
            DrawRectangleRounded(textBox, 2.0f, 8, RED);
        }
    }

    return PROJECT_MANAGER_WINDOW_MODE_CREATE;
}

char *HandleProjectManager()
{
    Font font = LoadFontFromMemory(".ttf", arialbd_ttf, arialbd_ttf_len, 256, NULL, 0);
    Font fontRE = LoadFontFromMemory(".ttf", sonsie_ttf, sonsie_ttf_len, 256, NULL, 0);
    if (font.texture.id == 0 || fontRE.texture.id == 0)
    {
        exit(1);
    }

    ProjectManagerWindowMode windowMode = PROJECT_MANAGER_WINDOW_MODE_MAIN;
    char *projectFileName = malloc(MAX_FILE_NAME * sizeof(char));
    projectFileName[0] = '\0';

    while (1)
    {
        switch (windowMode)
        {
        case PROJECT_MANAGER_WINDOW_MODE_MAIN:
            windowMode = MainWindow(font, fontRE);
            break;
        case PROJECT_MANAGER_WINDOW_MODE_LOAD:
            windowMode = WindowLoadProject(projectFileName, font);
            break;
        case PROJECT_MANAGER_WINDOW_MODE_CREATE:
            windowMode = WindowCreateProject(projectFileName, font);
            break;
        default:
            UnloadFont(font);
            UnloadFont(fontRE);

            return projectFileName;
        }

        EndDrawing();
    }
}