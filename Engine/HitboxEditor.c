#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "HitboxEditor.h"

#define SNAP_DIST 10.0f
#define MAX_VERTICES 64

HitboxEditorContext InitHitboxEditor(Texture2D tex, Vector2 pos, Vector2 scale)
{
    HitboxEditorContext hbEd = {0};
    hbEd.texture = tex;
    hbEd.position = pos;
    hbEd.poly.count = 0;
    hbEd.poly.isClosed = false;
    hbEd.draggingVerticeIndex = -1;
    hbEd.scale = scale;
    hbEd.lastActionType = HBED_LAST_ACTION_TYPE_NONE;
    return hbEd;
}

static bool IsNear(Vector2 a, Vector2 b, float dist)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return (dx * dx + dy * dy) < (dist * dist);
}

bool UpdateHitboxEditor(HitboxEditorContext *hbEd, Vector2 mouseLocal, GraphContext *graph, int hitboxEditingPinID)
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (hbEd->poly.isClosed)
        {
            for (int i = 0; i < hbEd->poly.count; i++)
            {
                hbEd->poly.vertices[i].x /= hbEd->scale.x;
                hbEd->poly.vertices[i].y /= hbEd->scale.y;
            }
            for (int i = 0; i < graph->pinCount; i++)
            {
                if (graph->pins[i].id == hitboxEditingPinID)
                {
                    graph->pins[i].hitbox = hbEd->poly;
                }
            }
            UnloadTexture(hbEd->texture);
            hbEd->texture.id = 0;
        }
        return false;
    }
    if (IsKeyPressed(KEY_R))
    {
        hbEd->poly.count = 0;
        hbEd->poly.isClosed = false;
        hbEd->hasChanged = true;
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z))
    {
        switch (hbEd->lastActionType)
        {
        case HBED_LAST_ACTION_TYPE_ADD:
            if (hbEd->poly.count != 0)
            {
                hbEd->poly.count--;
                hbEd->poly.isClosed = false;
                hbEd->hasChanged = true;
            }
            break;
        case HBED_LAST_ACTION_TYPE_DELETE:
            hbEd->poly.count++;
            for (int j = hbEd->poly.count - 1; j > hbEd->lastActionVerticeIndex; j--)
            {
                hbEd->poly.vertices[j] = hbEd->poly.vertices[j - 1];
            }
            hbEd->poly.vertices[hbEd->lastActionVerticeIndex] = hbEd->lastActionVertice;
            hbEd->hasChanged = true;
            break;
        case HBED_LAST_ACTION_TYPE_MOVE:
            hbEd->poly.vertices[hbEd->lastActionVerticeIndex].x -= hbEd->lastActionVertice.x;
            hbEd->poly.vertices[hbEd->lastActionVerticeIndex].y -= hbEd->lastActionVertice.y;
            hbEd->hasChanged = true;
            break;
        }

        hbEd->lastActionType = HBED_LAST_ACTION_TYPE_NONE;
    }

    if (hbEd->poly.isClosed)
    {

        if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            hbEd->draggingVerticeIndex = -1;
            return true;
        }

        if (hbEd->draggingVerticeIndex == -1)
        {
            for (int i = 0; i < hbEd->poly.count; i++)
            {
                Vector2 vertice = {
                    hbEd->position.x + hbEd->poly.vertices[i].x,
                    hbEd->position.y + hbEd->poly.vertices[i].y};

                if (IsNear(mouseLocal, vertice, SNAP_DIST))
                {
                    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                    {
                        hbEd->draggingVerticeIndex = i;
                        hbEd->lastActionType = HBED_LAST_ACTION_TYPE_MOVE;
                        hbEd->lastActionVerticeIndex = i;
                        hbEd->lastActionVertice = (Vector2){0, 0};
                        hbEd->hasChanged = true;
                    }
                    else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                    {
                        hbEd->lastActionType = HBED_LAST_ACTION_TYPE_DELETE;
                        hbEd->lastActionVertice = (Vector2){hbEd->poly.vertices[i].x, hbEd->poly.vertices[i].y};
                        hbEd->lastActionVerticeIndex = i;
                        for (int j = i; j < hbEd->poly.count - 1; j++)
                        {
                            hbEd->poly.vertices[j] = hbEd->poly.vertices[j + 1];
                        }
                        hbEd->poly.count--;
                        hbEd->hasChanged = true;
                    }
                }
            }

            if (hbEd->draggingVerticeIndex == -1)
            {
                return true;
            }
        }

        hbEd->poly.vertices[hbEd->draggingVerticeIndex].x += GetMouseDelta().x;
        hbEd->poly.vertices[hbEd->draggingVerticeIndex].y += GetMouseDelta().y;

        hbEd->lastActionVertice.x += GetMouseDelta().x;
        hbEd->lastActionVertice.y += GetMouseDelta().y;
    }
    else
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 rel = {
                mouseLocal.x - hbEd->position.x,
                mouseLocal.y - hbEd->position.y};

            if (hbEd->poly.count > 2)
            {
                Vector2 firstScreen = {
                    hbEd->position.x + hbEd->poly.vertices[0].x,
                    hbEd->position.y + hbEd->poly.vertices[0].y};
                if (IsNear(mouseLocal, firstScreen, SNAP_DIST))
                {
                    hbEd->poly.isClosed = true;
                    return true;
                }
            }

            if (hbEd->poly.count < MAX_VERTICES && !hbEd->poly.isClosed)
            {
                hbEd->poly.vertices[hbEd->poly.count++] = rel;

                hbEd->lastActionType = HBED_LAST_ACTION_TYPE_ADD;
                hbEd->lastActionVerticeIndex = hbEd->poly.count;
                hbEd->lastActionVertice = rel;
            }

            hbEd->hasChanged = true;
        }
    }

    return true;
}

void DrawHitboxEditor(HitboxEditorContext *hbEd, Vector2 mouseLocal)
{
    ClearBackground(COLOR_HE_BACKGROUND);

    DrawTexture(
        hbEd->texture,
        (int)(hbEd->position.x - hbEd->texture.width / 2),
        (int)(hbEd->position.y - hbEd->texture.height / 2),
        WHITE);

    for (int i = 0; i < hbEd->poly.count; i++)
    {
        Vector2 p = {
            hbEd->position.x + hbEd->poly.vertices[i].x,
            hbEd->position.y + hbEd->poly.vertices[i].y};
        DrawCircle(p.x, p.y, 4, GREEN);

        if (i > 0)
        {
            Vector2 prev = {
                hbEd->position.x + hbEd->poly.vertices[i - 1].x,
                hbEd->position.y + hbEd->poly.vertices[i - 1].y};
            DrawLineEx(prev, p, 2, WHITE);
        }
    }

    if (hbEd->poly.isClosed && hbEd->poly.count > 2)
    {
        Vector2 first = {
            hbEd->position.x + hbEd->poly.vertices[0].x,
            hbEd->position.y + hbEd->poly.vertices[0].y};
        Vector2 last = {
            hbEd->position.x + hbEd->poly.vertices[hbEd->poly.count - 1].x,
            hbEd->position.y + hbEd->poly.vertices[hbEd->poly.count - 1].y};
        DrawLineEx(last, first, 2, WHITE);

        for (int i = 0; i < hbEd->poly.count; i++)
        {
            Vector2 vertice = {
                hbEd->position.x + hbEd->poly.vertices[i].x,
                hbEd->position.y + hbEd->poly.vertices[i].y};

            if (IsNear(mouseLocal, vertice, SNAP_DIST))
            {
                DrawCircleLines(vertice.x, vertice.y, SNAP_DIST, GREEN);
            }
        }
    }
    else if (hbEd->poly.count > 0)
    {
        Vector2 last = {
            hbEd->position.x + hbEd->poly.vertices[hbEd->poly.count - 1].x,
            hbEd->position.y + hbEd->poly.vertices[hbEd->poly.count - 1].y};

        DrawLineEx(last, mouseLocal, 1, GRAY);

        if (hbEd->poly.count >= 3)
        {
            Vector2 first = {
                hbEd->position.x + hbEd->poly.vertices[0].x,
                hbEd->position.y + hbEd->poly.vertices[0].y};
            if (IsNear(mouseLocal, first, SNAP_DIST))
            {
                DrawCircleLines(first.x, first.y, SNAP_DIST, GREEN);
            }
        }
    }
}