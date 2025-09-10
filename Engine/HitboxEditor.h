#pragma once

#include "raylib.h"
#include <stdbool.h>
#include "Nodes.h"
#include "definitions.h"

typedef enum{
    HBED_LAST_ACTION_TYPE_NONE,
    HBED_LAST_ACTION_TYPE_ADD,
    HBED_LAST_ACTION_TYPE_DELETE,
    HBED_LAST_ACTION_TYPE_MOVE
}HitboxEditorLastActionType;

typedef struct {
    Texture2D texture;
    Vector2 position;
    Polygon poly;
    int draggingVerticeIndex;
    Vector2 scale;
    HitboxEditorLastActionType lastActionType;
    int lastActionVerticeIndex;
    Vector2 lastActionVertice;
    bool hasChanged;
} HitboxEditorContext;

HitboxEditorContext InitHitboxEditor(Texture2D tex, Vector2 pos, Vector2 scale);
bool UpdateHitboxEditor(HitboxEditorContext *e, Vector2 mouseLocal, GraphContext *graph, int hitboxEditingPinID);
void DrawHitboxEditor(HitboxEditorContext *e, Vector2 mouseLocal);