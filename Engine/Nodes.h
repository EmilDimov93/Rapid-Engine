#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "InfoByType.h"
#include "definitions.h"

#define MAX_NODE_PINS 16

extern const char *InputsByNodeTypes[][5];

extern const char *OutputsByNodeTypes[][5];

typedef struct Node
{
    int id;
    char name[MAX_VARIABLE_NAME_SIZE];
    NodeType type;
    Vector2 position;

    int inputPins[MAX_NODE_PINS];
    int inputCount;

    int outputPins[MAX_NODE_PINS];
    int outputCount;
} Node;

typedef struct Pin
{
    int id;
    PinType type;
    int nodeID;
    int posInNode;
    bool isInput;
    Vector2 position;
    bool isFloat;

    union{
        int pickedOption;
        char textFieldValue[128];
        Polygon hitbox;
    };
} Pin;

typedef struct Link
{
    int inputPinID;
    int outputPinID;
} Link;

#define INVALID_PIN (Pin) {-1}

typedef struct GraphContext
{
    Node *nodes;
    int nodeCount;
    int nextNodeID;

    Pin *pins;
    int pinCount;
    int nextPinID;

    Link *links;
    int linkCount;
    int nextLinkID;

    char **variables;
    NodeType *variableTypes;
    int variablesCount;
} GraphContext;

GraphContext InitGraphContext();

void FreeGraphContext(GraphContext *graph);

char *AssignAvailableVarName(GraphContext *graph, const char *baseName);

int SaveGraphToFile(const char *filename, GraphContext *graph);

bool LoadGraphFromFile(const char *filename, GraphContext *graph);

Pin CreatePin(GraphContext *graph, int nodeID, bool isInput, PinType type, int index, Vector2 pos);

bool CreateNode(GraphContext *graph, NodeType type, Vector2 pos);

bool DuplicateNode(GraphContext *graph, const Node *src, Vector2 pos);

void CreateLink(GraphContext *graph, Pin Pin1, Pin Pin2);

void DeleteNode(GraphContext *graph, int nodeID);

int FindPinIndexByID(GraphContext *graph, int id);

void RemoveConnections(GraphContext *graph, int pinID);