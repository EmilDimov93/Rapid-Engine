#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#include "Nodes.h"
#include "definitions.h"

#define MAX_LINKS_PER_PIN 16

#define MAX_SOUNDS 32

typedef struct RuntimePin
{
    int id;
    PinType type;
    int nodeIndex;
    bool isInput;
    int valueIndex;

    int nextNodeIndex;
    int pickedOption;
    char textFieldValue[256];
    int componentIndex;
} RuntimePin;

typedef struct RuntimeNode
{
    int index;
    NodeType type;

    RuntimePin *inputPins[MAX_NODE_PINS];
    int inputCount;

    RuntimePin *outputPins[MAX_NODE_PINS];
    int outputCount;
} RuntimeNode;

typedef struct RuntimeGraphContext
{
    RuntimeNode *nodes;
    int nodeCount;

    RuntimePin *pins;
    int pinCount;
} RuntimeGraphContext;

typedef enum
{
    VAL_NULL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_COLOR,
    VAL_SPRITE
} ValueType;

#ifndef POLYGON_H
#define POLYGON_H

#endif

typedef enum {
    HITBOX_NONE,
    HITBOX_RECT,
    HITBOX_CIRCLE,
    HITBOX_POLY
} HitboxType;

typedef struct {
    HitboxType type;
    Vector2 offset;
    union{
        Vector2 rectHitboxSize;
        float circleHitboxRadius;
        Polygon polygonHitbox;
    };
} Hitbox;

typedef struct
{
    bool isVisible;

    Vector2 position;
    int width;
    int height;
    float rotation;
    int layer;

    Hitbox hitbox;

    Texture2D texture;
} Sprite;

typedef enum
{
    PROP_TEXTURE,
    PROP_RECTANGLE,
    PROP_CIRCLE
} PropType;

typedef struct
{
    PropType propType;
    Vector2 position;
    int width;
    int height;

    float rotation; // not implemented rotation for props

    Color color;
    int layer;

    Hitbox hitbox;

    Texture2D texture;
} Prop;

typedef struct
{
    bool isVisible;
    bool isSprite;

    union
    {
        Sprite sprite;
        Prop prop;
    };
} SceneComponent;

typedef struct
{
    char *name;
    ValueType type;
    bool isVariable;
    union
    {
        float number;
        bool boolean;
        char *string;
        Vector2 vector;
        Color color;
        Sprite sprite;
    };
    int componentIndex;
} Value;

typedef struct
{
    int id;
    int componentIndex;
    int pixelsPerSecond;
    int angle;
    float duration;
}Force;

typedef struct
{
    Sound sound;
    float timeLeft;
}ActiveSound;

typedef struct
{
    Value *values;
    int valueCount;

    int *varIndexes;
    int varCount;
    
    Force *forces;
    int forceCount;

    SceneComponent *components;
    int componentCount;

    ActiveSound sounds[MAX_SOUNDS];
    int soundCount;

    char *projectPath;

    int loopNodeIndex;

    bool isFirstFrame;

    bool newLogMessage;
    char logMessages[MAX_LOG_MESSAGES][128];
    LogLevel logMessageLevels[MAX_LOG_MESSAGES];
    int logMessageCount;

    int *onButtonNodeIndexes;
    int onButtonNodeIndexesCount;

    Color backgroundColor;

    int fps;

    bool isInfiniteLoopProtectionOn;

    bool buildFailed;
    bool buildErrorOccured;

    bool shouldShowHitboxes;

    bool isPaused;

    float zoom;

    Vector2 cameraOffset;

    RuntimeGraphContext *runtimeGraph;
} InterpreterContext;

typedef enum{
    SPECIAL_VALUE_ERROR,
    SPECIAL_VALUE_MOUSE_X,
    SPECIAL_VALUE_MOUSE_Y,
    SPECIAL_VALUE_SCREEN_WIDTH,
    SPECIAL_VALUE_SCREEN_HEIGHT,
    SPECIAL_VALUE_CAMERA_CENTER_X,
    SPECIAL_VALUE_CAMERA_CENTER_Y,
    SPECIAL_VALUES_COUNT
}SpecialValuesInList;

typedef enum{
    COMPONENT_LAYER_NO_COLLISION,
    COMPONENT_LAYER_COLLISION_EVENTS,
    COMPONENT_LAYER_BLOCKING,
    COMPONENT_LAYER_COLLISION_EVENTS_AND_BLOCKING,
    COMPONENT_LAYER_COUNT
}ComponentLayers;

typedef enum{
    COLLISION_RESULT_NONE,
    COLLISION_RESULT_EVENT,
    COLLISION_RESULT_BLOCKING,
    COLLISION_RESULT_EVENT_AND_BLOCKING,
    COLLISION_RESULT_COUNT
}CollisionResult;

InterpreterContext InitInterpreterContext();

void FreeInterpreterContext(InterpreterContext *interpreter);

char *ValueTypeToString(ValueType type);

char *ValueToString(Value value);

RuntimeGraphContext ConvertToRuntimeGraph(GraphContext *graph, InterpreterContext *interpreter);

bool HandleGameScreen(InterpreterContext *interpreter, RuntimeGraphContext *graph, Vector2 mousePos, Rectangle screenBoundary);