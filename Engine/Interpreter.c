#include "Interpreter.h"
#include "raymath.h"

#define MAX_FORCES 100

InterpreterContext InitInterpreterContext()
{
    InterpreterContext intp = {0};

    intp.valueCount = 0;
    intp.varCount = 0;
    intp.onButtonNodeIndexesCount = 0;
    intp.componentCount = 0;
    intp.forceCount = 0;
    intp.tickNodeIndexesCount = 0;
    intp.soundCount = 0;

    intp.isFirstFrame = true;

    intp.newLogMessage = false;

    intp.buildFailed = false;
    intp.buildErrorOccured = false;

    intp.isInfiniteLoopProtectionOn = true;

    intp.backgroundColor = BLACK;

    intp.fps = 60;

    intp.shouldShowHitboxes = false;

    intp.isPaused = false;

    intp.cameraOffset = (Vector2){0, 0};
    intp.shakeCameraTimeRemaining = 0.0f;
    intp.shakeCameraIntensity = 0;

    intp.zoom = 1.0f;

    intp.shouldBreakFromLoop = false;

    intp.isSoundOn = true;
    intp.hasSoundOnChanged = true;

    return intp;
}

void FreeRuntimeGraphContext(RuntimeGraphContext *rg)
{
    if (!rg)
    {
        return;
    }

    if (rg->nodes)
    {
        free(rg->nodes);
        rg->nodes = NULL;
    }

    if (rg->pins)
    {
        for (int i = 0; i < rg->pinCount; i++)
        {
            if (rg->pins[i].textFieldValue)
            {
                free(rg->pins[i].textFieldValue);
            }
        }
        free(rg->pins);
        rg->pins = NULL;
    }

    rg->nodeCount = 0;
    rg->pinCount = 0;
}

void FreeInterpreterContext(InterpreterContext *intp)
{
    if (!intp)
        return;

    if (intp->values)
    {
        for (int i = 0; i < intp->valueCount; i++)
        {
            if (intp->values[i].type == VAL_STRING && intp->values[i].string)
            {
                free((void *)intp->values[i].string);
            }
        }
        free(intp->values);
    }

    free(intp->onButtonNodeIndexes);

    free(intp->forces);

    if (intp->components)
    {
        for (int i = 0; i < intp->componentCount; i++)
        {
            if (intp->components[i].isSprite && intp->components[i].sprite.texture.id)
            {
                UnloadTexture(intp->components[i].sprite.texture);
            }
        }
        free(intp->components);
    }

    free(intp->varIndexes);

    for (int i = 0; i < intp->soundCount; i++)
    {
        UnloadSound(intp->sounds[i].sound);
    }

    if (intp->runtimeGraph)
    {
        FreeRuntimeGraphContext(intp->runtimeGraph);
    }

    char *projectPath = intp->projectPath;
    *intp = InitInterpreterContext();
    intp->projectPath = projectPath;
}

char *ValueTypeToString(ValueType type)
{
    switch (type)
    {
    case VAL_NULL:
        return "null(Error)";
    case VAL_NUMBER:
        return "number";
    case VAL_STRING:
        return "string";
    case VAL_BOOL:
        return "boolean";
    case VAL_COLOR:
        return "color";
    case VAL_SPRITE:
        return "sprite";
    default:
        return "Error";
    }
}

char *ValueToString(Value value)
{
    static char temp[MAX_LOG_MESSAGE_SIZE];
    switch (value.type)
    {
    case VAL_NULL:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "Error");
        break;
    case VAL_NUMBER:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "%.2f", value.number);
        break;
    case VAL_STRING:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "%s", value.string);
        break;
    case VAL_BOOL:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "%s", value.boolean ? "true" : "false");
        break;
    case VAL_COLOR:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "R:%d G:%d B:%d A:%d", value.color.r, value.color.g, value.color.b, value.color.a);
        break;
    case VAL_SPRITE:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "%s, PosX: %.0f, PosY: %.0f, Rotation: %.2f", value.sprite.isVisible ? "Visible" : "Not visible", value.sprite.position.x, value.sprite.position.y, value.sprite.rotation);
        break;
    default:
        strmac(temp, MAX_LOG_MESSAGE_SIZE, "Error");
    }
    return temp;
}

void AddToLogFromInterpreter(InterpreterContext *intp, Value message, int level)
{
    if (intp->logMessageCount >= MAX_LOG_MESSAGES)
    {
        return;
    }

    strmac(intp->logMessages[intp->logMessageCount], MAX_LOG_MESSAGE_SIZE, "%s", ValueToString(message));
    intp->logMessageLevels[intp->logMessageCount] = level;
    intp->logMessageCount++;
    intp->newLogMessage = true;
}

void UpdateSpecialValues(InterpreterContext *intp, Vector2 mousePos, Rectangle screenBoundary)
{
    intp->values[SPECIAL_VALUE_MOUSE_X].number = mousePos.x;
    intp->values[SPECIAL_VALUE_MOUSE_Y].number = mousePos.y;
    intp->values[SPECIAL_VALUE_SCREEN_WIDTH].number = screenBoundary.width;
    intp->values[SPECIAL_VALUE_SCREEN_HEIGHT].number = screenBoundary.height;
    intp->values[SPECIAL_VALUE_CAMERA_CENTER_X].number = screenBoundary.x + screenBoundary.width / 2;
    intp->values[SPECIAL_VALUE_CAMERA_CENTER_Y].number = screenBoundary.y + screenBoundary.height / 2;
}

RuntimeGraphContext ConvertToRuntimeGraph(GraphContext *graph, InterpreterContext *intp)
{
    RuntimeGraphContext runtime = {0};

    runtime.nodeCount = graph->nodeCount;
    runtime.nodes = malloc(sizeof(RuntimeNode) * graph->nodeCount);

    if (!runtime.nodes)
    {
        intp->buildFailed = true;
        intp->buildErrorOccured = true;
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of memory: nodes{I200}"}, LOG_LEVEL_ERROR);
        return runtime;
    }

    runtime.pinCount = graph->pinCount;
    runtime.pins = malloc(sizeof(RuntimePin) * graph->pinCount);

    if (!runtime.pins)
    {
        free(runtime.nodes);
        runtime.nodes = NULL;
        intp->buildFailed = true;
        intp->buildErrorOccured = true;
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of memory: pins{I201}"}, LOG_LEVEL_ERROR);
        return runtime;
    }

    for (int i = 0; i < graph->pinCount; i++)
    {
        Pin *src = &graph->pins[i];
        RuntimePin *dst = &runtime.pins[i];

        dst->id = src->id;
        dst->type = src->type;
        dst->nodeIndex = -1;
        dst->isInput = src->isInput;
        dst->valueIndex = -1;
        dst->pickedOption = src->pickedOption;
        dst->nextNodeIndex = -1;
        dst->componentIndex = -1;
        dst->textFieldValue = strmac(NULL, MAX_LITERAL_NODE_FIELD_SIZE - 1, "%s", src->textFieldValue);
    }

    for (int i = 0; i < graph->nodeCount; i++)
    {
        Node *srcNode = &graph->nodes[i];
        RuntimeNode *dstNode = &runtime.nodes[i];

        dstNode->index = i;
        dstNode->type = srcNode->type;
        dstNode->inputCount = srcNode->inputCount;
        dstNode->outputCount = srcNode->outputCount;

        if (srcNode->type == NODE_FLIP_FLOP)
        {
            dstNode->flipFlopState = false;
        }

        for (int j = 0; j < srcNode->inputCount; j++)
        {
            int pinIndex = -1;
            for (int k = 0; k < graph->pinCount; k++)
            {
                if (graph->pins[k].id == srcNode->inputPins[j])
                {
                    pinIndex = k;
                    break;
                }
            }
            if (pinIndex < 0)
            {
                dstNode->inputPins[j] = NULL;
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Input pin mapping failed{I202}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            dstNode->inputPins[j] = &runtime.pins[pinIndex];
            runtime.pins[pinIndex].nodeIndex = i;
        }

        for (int j = 0; j < srcNode->outputCount; j++)
        {
            int pinIndex = -1;
            for (int k = 0; k < graph->pinCount; k++)
            {
                if (graph->pins[k].id == srcNode->outputPins[j])
                {
                    pinIndex = k;
                    break;
                }
            }
            if (pinIndex < 0)
            {
                dstNode->outputPins[j] = NULL;
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Output pin mapping failed{I203}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            dstNode->outputPins[j] = &runtime.pins[pinIndex];
            runtime.pins[pinIndex].nodeIndex = i;
        }
    }

    int totalOutputPins = 0;
    int totalComponents = 0;
    for (int i = 0; i < graph->nodeCount; i++)
    {
        RuntimeNode *node = &runtime.nodes[i];
        for (int j = 0; j < node->outputCount; j++)
        {
            if (node->outputPins[j] == NULL)
            {
                continue;
            }
            if (node->outputPins[j]->type != PIN_FLOW)
            {
                totalOutputPins++;
            }
        }

        if (node->type == NODE_CREATE_SPRITE || node->type == NODE_DRAW_PROP_TEXTURE || node->type == NODE_DRAW_PROP_RECTANGLE || node->type == NODE_DRAW_PROP_CIRCLE)
        {
            totalComponents++;
        }
    }

    int expectedValues = totalOutputPins + SPECIAL_VALUES_COUNT;
    intp->values = calloc(expectedValues, sizeof(Value));
    if (!intp->values)
    {
        intp->buildFailed = true;
        intp->buildErrorOccured = true;
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of memory: values{I204}"}, LOG_LEVEL_ERROR);
        return runtime;
    }

    intp->values[SPECIAL_VALUE_ERROR] = (Value){.type = VAL_STRING, .string = strmac(NULL, 12, "Error value"), .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Error value")};
    intp->values[SPECIAL_VALUE_MOUSE_X] = (Value){.type = VAL_NUMBER, .number = 0, .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Mouse X")};
    intp->values[SPECIAL_VALUE_MOUSE_Y] = (Value){.type = VAL_NUMBER, .number = 0, .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Mouse Y")};
    intp->values[SPECIAL_VALUE_SCREEN_WIDTH] = (Value){.type = VAL_NUMBER, .number = 0, .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Screen Width")};
    intp->values[SPECIAL_VALUE_SCREEN_HEIGHT] = (Value){.type = VAL_NUMBER, .number = 0, .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Screen Height")};
    intp->values[SPECIAL_VALUE_CAMERA_CENTER_X] = (Value){.type = VAL_NUMBER, .number = 0, .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Screen Center X")};
    intp->values[SPECIAL_VALUE_CAMERA_CENTER_Y] = (Value){.type = VAL_NUMBER, .number = 0, .name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "Screen Center Y")};
    intp->valueCount = SPECIAL_VALUES_COUNT;

    intp->components = calloc(totalComponents + 1, sizeof(SceneComponent));
    if (!intp->components)
    {
        intp->buildFailed = true;
        intp->buildErrorOccured = true;
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of memory: components{I205}"}, LOG_LEVEL_ERROR);
        return runtime;
    }
    intp->componentCount = 0;

    intp->varIndexes = malloc(sizeof(int) * (totalOutputPins + 1));
    if (!intp->varIndexes)
    {
        intp->buildFailed = true;
        intp->buildErrorOccured = true;
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of memory: varIndexes{I206}"}, LOG_LEVEL_ERROR);
        return runtime;
    }
    intp->varCount = 0;

    intp->forces = calloc(MAX_FORCES, sizeof(Force));
    if (!intp->forces)
    {
        intp->buildFailed = true;
        intp->buildErrorOccured = true;
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of memory: forces{I207}"}, LOG_LEVEL_ERROR);
        return runtime;
    }
    intp->forceCount = 0;

    for (int i = 0; i < graph->nodeCount; i++)
    {
        RuntimeNode *node = &runtime.nodes[i];
        Node *srcNode = &graph->nodes[i];

        switch (node->type)
        {
        case NODE_LITERAL_NUMBER:
            if (!node->inputPins[0])
            {
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Missing input for literal node{I208}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            intp->values[intp->valueCount].number = strtof(node->inputPins[0]->textFieldValue, NULL);
            intp->values[intp->valueCount].type = VAL_NUMBER;
            intp->values[intp->valueCount].isVariable = false;
            intp->values[intp->valueCount].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
            if (node->outputPins[0])
                node->outputPins[0]->valueIndex = intp->valueCount;
            intp->valueCount++;
            continue;
        case NODE_LITERAL_STRING:
            if (!node->inputPins[0])
            {
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Missing input for literal node{I208}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            intp->values[intp->valueCount].string = strmac(NULL, MAX_LITERAL_NODE_FIELD_SIZE - 1, node->inputPins[0]->textFieldValue);
            intp->values[intp->valueCount].type = VAL_STRING;
            intp->values[intp->valueCount].isVariable = false;
            intp->values[intp->valueCount].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
            if (node->outputPins[0])
                node->outputPins[0]->valueIndex = intp->valueCount;
            intp->valueCount++;
            continue;
        case NODE_LITERAL_BOOL:
            if (!node->inputPins[0])
            {
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Missing input for literal node{I208}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            if (strcmp(node->inputPins[0]->textFieldValue, "true") == 0)
            {
                intp->values[intp->valueCount].boolean = true;
            }
            else
            {
                intp->values[intp->valueCount].boolean = false;
            }
            intp->values[intp->valueCount].type = VAL_BOOL;
            intp->values[intp->valueCount].isVariable = false;
            intp->values[intp->valueCount].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
            if (node->outputPins[0])
                node->outputPins[0]->valueIndex = intp->valueCount;
            intp->valueCount++;
            continue;
        case NODE_LITERAL_COLOR:
            if (!node->inputPins[0])
            {
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Missing input for literal node{I208}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            unsigned int hexValue;
            if (sscanf(node->inputPins[0]->textFieldValue, "%x", &hexValue) == 1)
            {
                Color color = {(hexValue >> 24) & 0xFF, (hexValue >> 16) & 0xFF, (hexValue >> 8) & 0xFF, hexValue & 0xFF};
                intp->values[intp->valueCount].color = color;
                intp->values[intp->valueCount].type = VAL_COLOR;
                intp->values[intp->valueCount].isVariable = false;
                intp->values[intp->valueCount].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
                if (node->outputPins[0])
                {
                    node->outputPins[0]->valueIndex = intp->valueCount;
                }
                intp->valueCount++;
            }
            else
            {
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Error: Invalid color{I209}"}, LOG_LEVEL_ERROR);
                return runtime;
            }
            continue;
        default:
            break;
        }

        for (int j = 0; j < node->outputCount; j++)
        {
            RuntimePin *pin = node->outputPins[j];
            if (!pin)
                continue;
            if (pin->type == PIN_FLOW)
                continue;

            int idx = intp->valueCount;

            bool isVariable = false;
            if (node->type == NODE_CREATE_NUMBER || node->type == NODE_CREATE_STRING || node->type == NODE_CREATE_BOOL || node->type == NODE_CREATE_COLOR || node->type == NODE_CREATE_SPRITE)
            {
                isVariable = true;
                if (node->inputPins[1])
                {
                    node->inputPins[1]->valueIndex = idx;
                }
            }

            switch (pin->type)
            {
            case PIN_NUM:
                intp->values[idx].number = 0;
                intp->values[idx].type = VAL_NUMBER;
                intp->values[idx].isVariable = isVariable;
                intp->values[idx].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
                break;
            case PIN_STRING:
                intp->values[idx].string = strmac(NULL, MAX_VARIABLE_NAME_SIZE, "null");
                intp->values[idx].type = VAL_STRING;
                intp->values[idx].isVariable = isVariable;
                intp->values[idx].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
                break;
            case PIN_BOOL:
                intp->values[idx].boolean = false;
                intp->values[idx].type = VAL_BOOL;
                intp->values[idx].isVariable = isVariable;
                intp->values[idx].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
                break;
            case PIN_COLOR:
                intp->values[idx].color = WHITE;
                intp->values[idx].type = VAL_COLOR;
                intp->values[idx].isVariable = isVariable;
                intp->values[idx].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
                break;
            case PIN_SPRITE:
                intp->values[idx].sprite = (Sprite){0};
                intp->values[idx].type = VAL_SPRITE;
                intp->values[idx].isVariable = isVariable;
                intp->values[idx].name = strmac(NULL, MAX_VARIABLE_NAME_SIZE, srcNode->name);
                break;
            default:
                break;
            }

            intp->values[idx].componentIndex = -1;

            if (isVariable)
            {
                intp->varIndexes[intp->varCount] = idx;
                intp->varCount++;
            }

            if (intp->valueCount > expectedValues)
            {
                intp->buildFailed = true;
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Value array overflow{I20A}"}, LOG_LEVEL_ERROR);
                return runtime;
            }

            pin->valueIndex = idx;
            intp->valueCount++;
        }
    }

    if (intp->varCount > 0)
    {
        int *tmp = realloc(intp->varIndexes, sizeof(int) * intp->varCount);
        if (tmp)
            intp->varIndexes = tmp;
    }

    for (int i = 0; i < graph->nodeCount; i++)
    {
        RuntimeNode *node = &runtime.nodes[i];
        Node *srcNode = &graph->nodes[i];

        bool valueFound = false;

        switch (graph->nodes[i].type)
        {
        case NODE_GET_VARIABLE:
            for (int j = 0; j < graph->nodeCount; j++)
            {
                if (j != i && runtime.nodes[i].inputPins[0] && runtime.nodes[i].inputPins[0]->pickedOption < 256 && strcmp(graph->variables[runtime.nodes[i].inputPins[0]->pickedOption], graph->nodes[j].name) == 0)
                {
                    if (runtime.nodes[j].outputPins[1])
                        node->outputPins[0]->valueIndex = runtime.nodes[j].outputPins[1]->valueIndex;
                    valueFound = true;
                    break;
                }
            }
            if (!valueFound)
            {
                if (node->outputPins[0])
                    node->outputPins[0]->valueIndex = 0;
            }
            continue;
        case NODE_SET_VARIABLE:
            for (int j = 0; j < graph->nodeCount; j++)
            {
                if (j != i && runtime.nodes[i].inputPins[1] && runtime.nodes[i].inputPins[1]->pickedOption < 256 && strcmp(graph->variables[runtime.nodes[i].inputPins[1]->pickedOption], graph->nodes[j].name) == 0)
                {
                    if (runtime.nodes[j].outputPins[1])
                        node->outputPins[1]->valueIndex = runtime.nodes[j].outputPins[1]->valueIndex;
                    valueFound = true;
                    break;
                }
            }
            if (!valueFound)
            {
                if (node->outputPins[1])
                    node->outputPins[1]->valueIndex = 0;
            }
            continue;
        case NODE_GET_SCREEN_WIDTH:
            if (node->outputPins[0])
            {
                node->outputPins[0]->valueIndex = SPECIAL_VALUE_SCREEN_WIDTH;
            }
            continue;
        case NODE_GET_SCREEN_HEIGHT:
            if (node->outputPins[0])
            {
                node->outputPins[0]->valueIndex = SPECIAL_VALUE_SCREEN_HEIGHT;
            }
            continue;
        case NODE_GET_MOUSE_POSITION:
            if (node->outputPins[0])
            {
                node->outputPins[0]->valueIndex = SPECIAL_VALUE_MOUSE_X;
            }
            if (node->outputPins[1])
            {
                node->outputPins[1]->valueIndex = SPECIAL_VALUE_MOUSE_Y;
            }
            continue;
        case NODE_GET_CAMERA_CENTER:
            if (node->outputPins[0])
            {
                node->outputPins[0]->valueIndex = SPECIAL_VALUE_CAMERA_CENTER_X;
            }
            if (node->outputPins[1])
            {
                node->outputPins[1]->valueIndex = SPECIAL_VALUE_CAMERA_CENTER_Y;
            }
            continue;
        default:
            continue;
        }
    }

    for (int i = 0; i < graph->linkCount; i++)
    {
        int inputIndex = -1;
        int outputIndex = -1;

        for (int j = 0; j < graph->pinCount && (inputIndex == -1 || outputIndex == -1); j++)
        {
            if (graph->pins[j].id == graph->links[i].inputPinID)
                inputIndex = j;
            if (graph->pins[j].id == graph->links[i].outputPinID)
                outputIndex = j;
        }

        if (inputIndex == -1 || outputIndex == -1)
        {
            intp->buildFailed = true;
            intp->buildErrorOccured = true;
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Link pin missing{I20B}"}, LOG_LEVEL_ERROR);
            return runtime;
        }

        RuntimePin *inputPin = &runtime.pins[inputIndex];
        RuntimePin *outputPin = &runtime.pins[outputIndex];

        inputPin->valueIndex = outputPin->valueIndex;

        if (inputPin->type == PIN_FLOW && outputPin->type == PIN_FLOW)
        {
            if (inputPin->nodeIndex >= 0)
                outputPin->nextNodeIndex = inputPin->nodeIndex;
        }
    }

    for (int i = 0; i < graph->nodeCount; i++)
    {
        RuntimeNode *node = &runtime.nodes[i];

        switch (node->type)
        {
        case NODE_CREATE_SPRITE:
        {
            intp->components[intp->componentCount].isSprite = true;
            intp->components[intp->componentCount].isVisible = false;
            int fileIndex = -1;
            int wIndex = -1;
            int hIndex = -1;

            if (node->inputPins[1])
            {
                fileIndex = node->inputPins[1]->valueIndex;
            }
            if (node->inputPins[2])
            {
                wIndex = node->inputPins[2]->valueIndex;
            }
            if (node->inputPins[3])
            {
                hIndex = node->inputPins[3]->valueIndex;
            }

            if (fileIndex != -1 && fileIndex < intp->valueCount && intp->values[fileIndex].string && intp->values[fileIndex].string[0])
            {
                char path[MAX_FILE_PATH];
                strmac(path, MAX_FILE_PATH, "%s%c%s", intp->projectPath, PATH_SEPARATOR, intp->values[fileIndex].string);
                Texture2D tex = LoadTexture(path);
                if (tex.id == 0)
                {
                    intp->buildErrorOccured = true;
                    AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Failed to load texture{I20C}"}, LOG_LEVEL_ERROR);
                    return runtime;
                }
                else
                {
                    intp->components[intp->componentCount].sprite.texture = tex;
                }
            }
            else
            {
                intp->buildErrorOccured = true;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Invalid texture input{I20D}"}, LOG_LEVEL_ERROR);
                return runtime;
            }

            if (wIndex != -1 && wIndex < intp->valueCount)
                intp->components[intp->componentCount].sprite.width = intp->values[wIndex].number;
            if (hIndex != -1 && hIndex < intp->valueCount)
                intp->components[intp->componentCount].sprite.height = intp->values[hIndex].number;
            if (node->inputPins[4]->pickedOption >= 0 && node->inputPins[4]->pickedOption < COMPONENT_LAYER_COUNT)
            {
                intp->components[intp->componentCount].sprite.layer = node->inputPins[4]->pickedOption;
            }

            intp->components[intp->componentCount].sprite.hitbox.type = HITBOX_POLY;

            for (int j = 0; j < graph->pinCount; j++)
            {
                if (graph->nodes[i].inputPins[5] && graph->pins[j].id == graph->nodes[i].inputPins[5])
                {
                    intp->components[intp->componentCount].sprite.hitbox.polygonHitbox = graph->pins[j].hitbox;
                }
            }

            if (node->outputPins[1])
                node->outputPins[1]->componentIndex = intp->componentCount;

            for (int j = 0; j < runtime.nodeCount; j++)
            {
                for (int k = 0; k < runtime.nodes[j].inputCount; k++)
                {
                    if (!runtime.nodes[j].inputPins[k])
                    {
                        continue;
                    }
                    if (runtime.nodes[j].inputPins[k]->type == PIN_SPRITE_VARIABLE && runtime.nodes[j].inputPins[k]->pickedOption != 0)
                    {
                        const char *varPtr = graph->nodes[i].name;
                        int picked = runtime.nodes[j].inputPins[k]->pickedOption - 1;
                        if (picked < 0 || picked >= intp->varCount)
                        {
                            continue;
                        }
                        const char *valPtr = intp->values[intp->varIndexes[picked]].name;

                        if (varPtr && valPtr)
                        {
                            char varName[MAX_VARIABLE_NAME_SIZE];
                            char valName[MAX_VARIABLE_NAME_SIZE];
                            strmac(varName, MAX_VARIABLE_NAME_SIZE, "%s", varPtr);
                            strmac(valName, MAX_VARIABLE_NAME_SIZE, "%s", valPtr);

                            if (strcmp(varName, valName) == 0)
                            {
                                intp->values[intp->varIndexes[picked]].componentIndex = intp->componentCount;
                                runtime.nodes[j].inputPins[k]->valueIndex = intp->varIndexes[picked];
                            }
                        }
                    }
                }
            }

            intp->componentCount++;
        }
        break;
        case NODE_DRAW_PROP_TEXTURE:
            continue;
        case NODE_DRAW_PROP_RECTANGLE:
            intp->components[intp->componentCount].isSprite = false;
            intp->components[intp->componentCount].isVisible = false;
            intp->components[intp->componentCount].prop.propType = PROP_RECTANGLE;
            if (node->inputPins[3] && node->inputPins[3]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.width = intp->values[node->inputPins[3]->valueIndex].number;
            if (node->inputPins[4] && node->inputPins[4]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.height = intp->values[node->inputPins[4]->valueIndex].number;
            if (node->inputPins[1] && node->inputPins[1]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.position.x = intp->values[node->inputPins[1]->valueIndex].number - intp->components[intp->componentCount].prop.width / 2;
            if (node->inputPins[2] && node->inputPins[2]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.position.y = intp->values[node->inputPins[2]->valueIndex].number - intp->components[intp->componentCount].prop.height / 2;
            if (node->inputPins[5] && node->inputPins[5]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.color = intp->values[node->inputPins[5]->valueIndex].color;
            if (node->inputPins[6] && node->inputPins[6]->pickedOption >= 0 && node->inputPins[6]->pickedOption < COMPONENT_LAYER_COUNT)
            {
                intp->components[intp->componentCount].prop.layer = node->inputPins[6]->pickedOption;
            }

            intp->components[intp->componentCount].prop.hitbox.type = HITBOX_RECT;
            intp->components[intp->componentCount].prop.hitbox.rectHitboxSize = (Vector2){intp->components[intp->componentCount].prop.width, intp->components[intp->componentCount].prop.height};
            intp->components[intp->componentCount].prop.hitbox.offset = (Vector2){0, 0};

            if (node->outputPins[1])
                node->outputPins[1]->componentIndex = intp->componentCount;
            intp->componentCount++;
            continue;
        case NODE_DRAW_PROP_CIRCLE:
            intp->components[intp->componentCount].isSprite = false;
            intp->components[intp->componentCount].isVisible = false;
            intp->components[intp->componentCount].prop.propType = PROP_CIRCLE;
            if (node->inputPins[1] && node->inputPins[1]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.position.x = intp->values[node->inputPins[1]->valueIndex].number;
            if (node->inputPins[2] && node->inputPins[2]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.position.y = intp->values[node->inputPins[2]->valueIndex].number;
            if (node->inputPins[3] && node->inputPins[3]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.width = intp->values[node->inputPins[3]->valueIndex].number * 2;
            if (node->inputPins[3] && node->inputPins[3]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.height = intp->values[node->inputPins[3]->valueIndex].number * 2;
            if (node->inputPins[4] && node->inputPins[4]->valueIndex < intp->valueCount)
                intp->components[intp->componentCount].prop.color = intp->values[node->inputPins[4]->valueIndex].color;
            if (node->inputPins[5] && node->inputPins[5]->pickedOption >= 0 && node->inputPins[5]->pickedOption < COMPONENT_LAYER_COUNT)
            {
                intp->components[intp->componentCount].prop.layer = node->inputPins[5]->pickedOption;
            }

            intp->components[intp->componentCount].prop.hitbox.type = HITBOX_CIRCLE;
            intp->components[intp->componentCount].prop.hitbox.circleHitboxRadius = intp->components[intp->componentCount].prop.width / 2;
            intp->components[intp->componentCount].prop.hitbox.offset = (Vector2){0, 0};

            if (node->outputPins[1])
                node->outputPins[1]->componentIndex = intp->componentCount;
            intp->componentCount++;
            continue;
        default:
            break;
        }
    }

    return runtime;
}

int DoesForceExist(InterpreterContext *intp, int id)
{
    for (int i = 0; i < intp->forceCount; i++)
    {
        if (intp->forces[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

void InterpretStringOfNodes(int lastNodeIndex, InterpreterContext *intp, RuntimeGraphContext *graph, int outFlowPinIndexInNode)
{
    if (lastNodeIndex < 0 || lastNodeIndex >= graph->nodeCount)
    {
        return;
    }

    if (graph->nodes[lastNodeIndex].outputCount == 0 || graph->nodes[lastNodeIndex].outputPins[outFlowPinIndexInNode]->nextNodeIndex == -1)
    {
        return;
    }

    int currNodeIndex = graph->nodes[lastNodeIndex].outputPins[outFlowPinIndexInNode]->nextNodeIndex;
    if (currNodeIndex < 0 || currNodeIndex >= graph->nodeCount)
    {
        return;
    }

    RuntimeNode *node = &graph->nodes[currNodeIndex];

    switch (node->type)
    {
    case NODE_UNKNOWN:
    {
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Unknown node{I20E}"}, LOG_LEVEL_ERROR);
        break;
    }

    case NODE_CREATE_NUMBER:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            float *numToSet = &intp->values[node->outputPins[1]->valueIndex].number;
            float newNum = intp->values[node->inputPins[1]->valueIndex].number;
            *numToSet = newNum;
        }
        break;
    }

    case NODE_CREATE_STRING:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            char **strToSet = &intp->values[node->outputPins[1]->valueIndex].string;
            char *newStr = intp->values[node->inputPins[1]->valueIndex].string;
            *strToSet = newStr;
        }
        break;
    }

    case NODE_CREATE_BOOL:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            bool *boolToSet = &intp->values[node->outputPins[1]->valueIndex].boolean;
            bool newBool = intp->values[node->inputPins[1]->valueIndex].boolean;
            *boolToSet = newBool;
        }
        break;
    }

    case NODE_CREATE_COLOR:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            Color *colorToSet = &intp->values[node->outputPins[1]->valueIndex].color;
            Color newColor = intp->values[node->inputPins[1]->valueIndex].color;
            *colorToSet = intp->values[node->inputPins[1]->valueIndex].color;
        }
        break;
    }

    case NODE_CAST_TO_NUMBER:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        Value val = intp->values[node->inputPins[1]->valueIndex];
        Value *newVal = &intp->values[node->outputPins[1]->valueIndex];

        switch (val.type)
        {
        case VAL_NUMBER:
            newVal->number = val.number;
            break;
        case VAL_STRING:
            newVal->number = strlen(val.string);
            break;
        case VAL_BOOL:
            newVal->number = val.boolean;
            break;
        case VAL_COLOR:
            newVal->number = 0;
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Can't cast Color to Number{I108}"}, LOG_LEVEL_WARNING);
            break;
        case VAL_SPRITE:
            newVal->number = 0;
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Can't cast Sprite to Number{I109}"}, LOG_LEVEL_WARNING);
            break;
        default:
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Unknown pin type{I113}"}, LOG_LEVEL_WARNING);
            break;
        }

        break;
    }

    case NODE_CAST_TO_STRING:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        Value val = intp->values[node->inputPins[1]->valueIndex];
        Value *newVal = &intp->values[node->outputPins[1]->valueIndex];

        switch (val.type)
        {
        case VAL_NUMBER:
            strmac(newVal->string, MAX_LITERAL_NODE_FIELD_SIZE, "%.2f", val.number);
            break;
        case VAL_STRING:
            strmac(newVal->string, MAX_LITERAL_NODE_FIELD_SIZE, "%s", val.string);
            break;
        case VAL_BOOL:
            strmac(newVal->string, MAX_LITERAL_NODE_FIELD_SIZE, "%s", val.boolean ? "true" : "false");
            break;
        case VAL_COLOR:
            strmac(newVal->string, MAX_LITERAL_NODE_FIELD_SIZE, "R:%d G:%d B:%d A:%d", val.color.r, val.color.g, val.color.b, val.color.a);
            break;
        case VAL_SPRITE:
            strmac(newVal->string, MAX_LOG_MESSAGE_SIZE, "%s, PosX: %.0f, PosY: %.0f, Rotation: %.2f", val.sprite.isVisible ? "Visible" : "Not visible", val.sprite.position.x, val.sprite.position.y, val.sprite.rotation);
            break;
        default:
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Unknown pin type{I113}"}, LOG_LEVEL_WARNING);
            break;
        }

        break;
    }

    case NODE_CAST_TO_BOOL:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        Value val = intp->values[node->inputPins[1]->valueIndex];
        Value *newVal = &intp->values[node->outputPins[1]->valueIndex];

        switch (val.type)
        {
        case VAL_NUMBER:
            newVal->boolean = val.number > 0;
            break;
        case VAL_STRING:
            newVal->boolean = strlen(val.string) > 0;
            break;
        case VAL_BOOL:
            newVal->boolean = val.boolean;
            break;
        case VAL_COLOR:
            newVal->boolean = val.color.a != 0;
            break;
        case VAL_SPRITE:
            newVal->boolean = val.sprite.isVisible;
            break;
        default:
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Unknown pin type{I113}"}, LOG_LEVEL_WARNING);
            break;
        }

        break;
    }

    case NODE_CAST_TO_COLOR:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        Value val = intp->values[node->inputPins[1]->valueIndex];
        Value *newVal = &intp->values[node->outputPins[1]->valueIndex];

        switch (val.type)
        {
        case VAL_NUMBER:
            newVal->color = BLACK;
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Can't cast Number to Color{I110}"}, LOG_LEVEL_WARNING);
            break;
        case VAL_STRING:
        {
            unsigned int hexValue;
            if (sscanf(val.string, "%x", &hexValue) == 1)
            {
                newVal->color = (Color){
                    (hexValue >> 24) & 0xFF,
                    (hexValue >> 16) & 0xFF,
                    (hexValue >> 8) & 0xFF,
                    hexValue & 0xFF};
            }
            else
            {
                newVal->color = BLACK;
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Error: Invalid color{I209}"}, LOG_LEVEL_ERROR);
            }
            break;
        }
        case VAL_BOOL:
            newVal->color = BLACK;
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Can't cast Bool to Color{I111}"}, LOG_LEVEL_WARNING);
            break;
        case VAL_COLOR:
            newVal->color = val.color;
            break;
        case VAL_SPRITE:
            newVal->color = BLACK;
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Can't cast Sprite to Color{I112}"}, LOG_LEVEL_WARNING);
            break;
        default:
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Unknown pin type{I113}"}, LOG_LEVEL_WARNING);
            break;
        }

        break;
    }

    case NODE_CREATE_CUSTOM_EVENT:
    {
        break;
    }

    case NODE_CALL_CUSTOM_EVENT:
    {
        break;
    }

    case NODE_GET_RANDOM_NUMBER:
    {
        if (node->inputPins[1]->valueIndex != -1 && node->inputPins[2]->valueIndex != -1)
        {
            if (node->outputPins[1]->valueIndex != -1)
            {
                intp->values[node->outputPins[1]->valueIndex].number = GetRandomValue((int)intp->values[node->inputPins[1]->valueIndex].number, (int)intp->values[node->inputPins[2]->valueIndex].number);
            }
        }
        break;
    }

    case NODE_GET_SPRITE_POSITION:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            if (node->outputPins[1]->valueIndex != -1 && node->outputPins[2]->valueIndex != -1)
            {
                intp->values[node->outputPins[1]->valueIndex].number = intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.position.x;
                intp->values[node->outputPins[2]->valueIndex].number = intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.position.y;
            }
        }
        break;
    }

    case NODE_SET_VARIABLE:
    {
        if (node->outputPins[1]->valueIndex != -1)
        {
            Value *valToSet = &intp->values[node->outputPins[1]->valueIndex];
            Value newValue = intp->values[node->inputPins[2]->valueIndex];
            switch (valToSet->type)
            {
            case VAL_NUMBER:
                valToSet->number = newValue.number;
                break;
            case VAL_STRING:
                valToSet->string = newValue.string;
                break;
            case VAL_BOOL:
                valToSet->boolean = newValue.boolean;
                break;
            case VAL_COLOR:
                valToSet->color = newValue.color;
                break;
            case VAL_SPRITE:
                valToSet->sprite = newValue.sprite;
                break;
            default:
                break;
            }
        }
        break;
    }

    case NODE_SET_BACKGROUND:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            intp->backgroundColor = intp->values[node->inputPins[1]->valueIndex].color;
        }
        break;
    }

    case NODE_SET_FPS:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            intp->fps = intp->values[node->inputPins[1]->valueIndex].number;
        }
        break;
    }

    case NODE_BRANCH:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            bool *condition = &intp->values[node->inputPins[1]->valueIndex].boolean;
            if (*condition)
            {
                InterpretStringOfNodes(currNodeIndex, intp, graph, 0);
            }
            else
            {
                InterpretStringOfNodes(currNodeIndex, intp, graph, 1);
            }
        }
        return;
    }

    case NODE_LOOP:
    {
        int steps = MAX_ITERATIONS_BEFORE_ILP;
        if (node->inputPins[1]->valueIndex != -1)
        {
            bool *condition = &intp->values[node->inputPins[1]->valueIndex].boolean;
            while (*condition)
            {
                if (steps == 0)
                {
                    if (intp->isInfiniteLoopProtectionOn)
                    {
                        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Possible infinite loop detected and exited! You can turn off infinite loop protection in settings{I210}"}, LOG_LEVEL_ERROR);
                        break;
                    }
                    else
                    {
                        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Possible infinite loop detected! Infinite loop protection is off!{I101}"}, LOG_LEVEL_WARNING);
                    }
                }
                else
                {
                    steps--;
                }
                InterpretStringOfNodes(currNodeIndex, intp, graph, 1);
                if (intp->shouldBreakFromLoop)
                {
                    intp->shouldBreakFromLoop = false;
                    break;
                }
            }
        }
        break;
    }

    case NODE_FLIP_FLOP:
    {
        if (node->flipFlopState)
        {
            InterpretStringOfNodes(currNodeIndex, intp, graph, 0);
        }
        else
        {
            InterpretStringOfNodes(currNodeIndex, intp, graph, 1);
        }

        node->flipFlopState = !node->flipFlopState;
        return;
        break;
    }

    case NODE_BREAK:
    {
        intp->shouldBreakFromLoop = true;
        return;
        break;
    }

    case NODE_SEQUENCE:
    {
        InterpretStringOfNodes(currNodeIndex, intp, graph, 0);
        InterpretStringOfNodes(currNodeIndex, intp, graph, 1);
        InterpretStringOfNodes(currNodeIndex, intp, graph, 2);
        return;
        break;
    }

    case NODE_CREATE_SPRITE:
    {
        if (node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        Sprite *sprite = &intp->values[node->outputPins[1]->valueIndex].sprite;
        if (node->inputPins[2]->valueIndex != -1)
        {
            sprite->width = intp->values[node->inputPins[2]->valueIndex].number;
        }
        if (node->inputPins[3]->valueIndex != -1)
        {
            sprite->height = intp->values[node->inputPins[3]->valueIndex].number;
        }
        if (node->inputPins[4]->pickedOption >= 0 && node->inputPins[4]->pickedOption < COMPONENT_LAYER_COUNT)
        {
            sprite->layer = node->inputPins[4]->pickedOption;
        }
        if (node->outputPins[1]->componentIndex != -1)
        {
            Texture2D tempTex = intp->components[node->outputPins[1]->componentIndex].sprite.texture;
            Polygon tempHitbox = intp->components[node->outputPins[1]->componentIndex].sprite.hitbox.polygonHitbox;
            intp->components[node->outputPins[1]->componentIndex].sprite = *sprite;
            intp->components[node->outputPins[1]->componentIndex].sprite.texture = tempTex;
            intp->components[node->outputPins[1]->componentIndex].sprite.hitbox.type = HITBOX_POLY;
            intp->components[node->outputPins[1]->componentIndex].sprite.hitbox.polygonHitbox = tempHitbox;
        }
        break;
    }

    case NODE_SPAWN_SPRITE:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].isVisible = true;
            if (node->inputPins[2]->valueIndex != -1)
            {
                intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.position.x = intp->values[node->inputPins[2]->valueIndex].number;
            }
            if (node->inputPins[3]->valueIndex != -1)
            {
                intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.position.y = intp->values[node->inputPins[3]->valueIndex].number;
            }
            if (node->inputPins[4]->valueIndex != -1)
            {
                intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.rotation = -1 * (intp->values[node->inputPins[4]->valueIndex].number - 360);
            }
        }
        break;
    }

    case NODE_DESTROY_SPRITE:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].isVisible = false;
        }
        break;
    }

    case NODE_SET_SPRITE_POSITION:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.position.x = intp->values[node->inputPins[2]->valueIndex].number;
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.position.y = intp->values[node->inputPins[3]->valueIndex].number;
        }
        break;
    }

    case NODE_SET_SPRITE_ROTATION:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.rotation = -1 * (intp->values[node->inputPins[2]->valueIndex].number - 360);
        }
        break;
    }

    case NODE_SET_SPRITE_TEXTURE:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            UnloadTexture(intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.texture);
            char path[MAX_FILE_PATH];
            strmac(path, MAX_FILE_PATH, "%s%c%s", intp->projectPath, PATH_SEPARATOR, intp->values[node->inputPins[2]->valueIndex].string);
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.texture = LoadTexture(path);
        }
        break;
    }

    case NODE_SET_SPRITE_SIZE:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
        {
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.width = intp->values[node->inputPins[2]->valueIndex].number;
            intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex].sprite.height = intp->values[node->inputPins[3]->valueIndex].number;
        }
        break;
    }

    case NODE_MOVE_TO_SPRITE:
    {
        break;
    }

    case NODE_FORCE_SPRITE:
    {
        if (intp->values[node->inputPins[1]->valueIndex].componentIndex != -1)
        {
            SceneComponent *component = &intp->components[intp->values[node->inputPins[1]->valueIndex].componentIndex];
            int forceIndex = DoesForceExist(intp, node->index);
            if (forceIndex != -1)
            {
                intp->forces[forceIndex].duration = intp->values[node->inputPins[4]->valueIndex].number;
            }
            else if (intp->values[node->inputPins[1]->valueIndex].componentIndex >= 0 && intp->values[node->inputPins[1]->valueIndex].componentIndex < intp->componentCount)
            {
                intp->forces[intp->forceCount].id = node->index;
                intp->forces[intp->forceCount].componentIndex = intp->values[node->inputPins[1]->valueIndex].componentIndex;
                intp->forces[intp->forceCount].pixelsPerSecond = intp->values[node->inputPins[2]->valueIndex].number;
                intp->forces[intp->forceCount].angle = intp->values[node->inputPins[3]->valueIndex].number;
                intp->forces[intp->forceCount].duration = intp->values[node->inputPins[4]->valueIndex].number;
                intp->forceCount++;
            }
        }
        break;
    }

    case NODE_DRAW_PROP_TEXTURE:
    {
        break;
    }

    case NODE_DRAW_PROP_RECTANGLE:
    {
        if (node->outputPins[1]->componentIndex != -1)
        {
            intp->components[node->outputPins[1]->componentIndex].isVisible = true;
        }
        break;
    }

    case NODE_DRAW_PROP_CIRCLE:
    {
        if (node->outputPins[1]->componentIndex != -1)
        {
            intp->components[node->outputPins[1]->componentIndex].isVisible = true;
        }
        break;
    }

    case NODE_COMPARISON:
    {
        if (node->inputPins[2]->valueIndex == -1 || node->inputPins[3]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }
        float numA = intp->values[node->inputPins[2]->valueIndex].number;
        float numB = intp->values[node->inputPins[3]->valueIndex].number;
        bool *result = &intp->values[node->outputPins[1]->valueIndex].boolean;
        switch (node->inputPins[1]->pickedOption)
        {
        case EQUAL_TO:
            *result = numA == numB;
            break;
        case GREATER_THAN:
            *result = numA > numB;
            break;
        case LESS_THAN:
            *result = numA < numB;
            break;
        default:
            break;
        }
        break;
    }

    case NODE_GATE:
    {
        if (node->inputPins[2]->valueIndex == -1 || node->inputPins[3]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }
        bool boolA = intp->values[node->inputPins[2]->valueIndex].boolean;
        bool boolB = intp->values[node->inputPins[3]->valueIndex].boolean;
        bool *result = &intp->values[node->outputPins[1]->valueIndex].boolean;
        switch (node->inputPins[1]->pickedOption)
        {
        case AND:
            *result = boolA && boolB;
            break;
        case OR:
            *result = boolA || boolB;
            break;
        case NOT:
            *result = !boolA;
            break;
        case XOR:
            *result = boolA != boolB;
            break;
        case NAND:
            *result = !(boolA && boolB);
            break;
        case NOR:
            *result = !(boolA || boolB);
            break;
        default:
            break;
        }
        break;
    }

    case NODE_ARITHMETIC:
    {
        if (node->inputPins[2]->valueIndex == -1 || node->inputPins[3]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }
        float numA = intp->values[node->inputPins[2]->valueIndex].number;
        float numB = intp->values[node->inputPins[3]->valueIndex].number;
        float *result = &intp->values[node->outputPins[1]->valueIndex].number;
        switch (node->inputPins[1]->pickedOption)
        {
        case ADD:
            *result = numA + numB;
            break;
        case SUBTRACT:
            *result = numA - numB;
            break;
        case MULTIPLY:
            *result = numA * numB;
            break;
        case DIVIDE:
            *result = numA / numB;
            break;
        case MODULO:
            *result = (int)numA % (int)numB;
            break;
        default:
            break;
        }
        break;
    }

    case NODE_CLAMP:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->inputPins[2]->valueIndex == -1 || node->inputPins[3]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        float num = intp->values[node->inputPins[1]->valueIndex].number;
        float min = intp->values[node->inputPins[2]->valueIndex].number;
        float max = intp->values[node->inputPins[3]->valueIndex].number;
        if (num < min)
        {
            num = min;
        }
        else if (num > max)
        {
            num = max;
        }

        intp->values[node->outputPins[1]->valueIndex].number = num;
        break;
    }

    case NODE_LERP:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->inputPins[2]->valueIndex == -1 || node->inputPins[3]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        float numA = intp->values[node->inputPins[1]->valueIndex].number;
        float numB = intp->values[node->inputPins[2]->valueIndex].number;
        float alpha = intp->values[node->inputPins[3]->valueIndex].number;
        if (numB < numA)
        {
            float temp = numA;
            numA = numB;
            numB = numA;
        }
        if (alpha < 0)
        {
            alpha = 0;
        }
        else if (alpha > 1.0f)
        {
            alpha = 1.0f;
        }

        intp->values[node->outputPins[1]->valueIndex].number = numA + (numB - numA) * alpha;
        break;
    }

    case NODE_SIN:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        intp->values[node->outputPins[1]->valueIndex].number = sin(intp->values[node->inputPins[1]->valueIndex].number);

        break;
    }

    case NODE_COS:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->outputPins[1]->valueIndex == -1)
        {
            break;
        }

        intp->values[node->outputPins[1]->valueIndex].number = cos(intp->values[node->inputPins[1]->valueIndex].number);

        break;
    }

    case NODE_PRINT_TO_LOG:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            AddToLogFromInterpreter(intp, intp->values[node->inputPins[1]->valueIndex], LOG_LEVEL_DEBUG);
        }
        break;
    }

    case NODE_DRAW_DEBUG_LINE:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->inputPins[2]->valueIndex == -1 || node->inputPins[3]->valueIndex == -1 || node->inputPins[4]->valueIndex == -1 || node->inputPins[5]->valueIndex == -1)
        {
            break;
        }
        DrawLine(
            intp->values[node->inputPins[1]->valueIndex].number,
            intp->values[node->inputPins[2]->valueIndex].number,
            intp->values[node->inputPins[3]->valueIndex].number,
            intp->values[node->inputPins[4]->valueIndex].number,
            intp->values[node->inputPins[5]->valueIndex].color);
        break;
    }

    case NODE_COMMENT:
    {
        break;
    }

    case NODE_MOVE_CAMERA:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            intp->cameraOffset.x += intp->values[node->inputPins[1]->valueIndex].number;
        }
        if (node->inputPins[2]->valueIndex != -1)
        {
            intp->cameraOffset.y += intp->values[node->inputPins[2]->valueIndex].number;
        }
        break;
    }
    case NODE_ZOOM_CAMERA:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            if (intp->zoom + intp->values[node->inputPins[1]->valueIndex].number >= MIN_ZOOM)
            {
                intp->zoom += intp->values[node->inputPins[1]->valueIndex].number;
            }
            else
            {
                intp->zoom = MIN_ZOOM;
            }
        }
        break;
    }
    case NODE_SHAKE_CAMERA:
    {
        if (node->inputPins[1]->valueIndex == -1 || node->inputPins[2]->valueIndex == -1)
        {
            break;
        }

        intp->shakeCameraIntensity = Clamp(intp->values[node->inputPins[1]->valueIndex].number, 0, 20);
        intp->shakeCameraTimeRemaining = intp->values[node->inputPins[2]->valueIndex].number;
        break;
    }
    case NODE_PLAY_SOUND:
    {
        if (node->inputPins[1]->valueIndex != -1)
        {
            if (intp->soundCount >= MAX_SOUNDS)
            {
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Maximum sounds reached{I104}"}, LOG_LEVEL_WARNING);
                break;
            }
            Sound temp = LoadSound(TextFormat("%s%c%s", intp->projectPath, PATH_SEPARATOR, intp->values[node->inputPins[1]->valueIndex].string));
            if (temp.frameCount > 0)
            {
                intp->sounds[intp->soundCount].sound = temp;
                intp->sounds[intp->soundCount].timeLeft = (float)intp->sounds[intp->soundCount].sound.frameCount / intp->sounds[intp->soundCount].sound.stream.sampleRate;
                PlaySound(intp->sounds[intp->soundCount].sound);
                intp->soundCount++;
            }
            else
            {
                UnloadSound(intp->sounds[intp->soundCount].sound);
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Invalid sound{I105}"}, LOG_LEVEL_WARNING);
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }

    if (currNodeIndex != lastNodeIndex)
    {
        InterpretStringOfNodes(currNodeIndex, intp, graph, 0);
    }
}

void DrawHitbox(Hitbox *h, Vector2 centerPos, Vector2 spriteSize, Vector2 texSize, Color color)
{
    float scaleX = spriteSize.x / texSize.x;
    float scaleY = spriteSize.y / texSize.y;

    switch (h->type)
    {
    case HITBOX_RECT:
    {
        Rectangle r = {
            centerPos.x + h->offset.x * scaleX,
            centerPos.y + h->offset.y * scaleY,
            h->rectHitboxSize.x * scaleX,
            h->rectHitboxSize.y * scaleY};
        DrawRectangleLinesEx(r, 1, color);
    }
    break;

    case HITBOX_CIRCLE:
    {
        Vector2 c = {
            centerPos.x + h->offset.x * scaleX,
            centerPos.y + h->offset.y * scaleY};
        DrawCircleLines((int)c.x, (int)c.y, h->circleHitboxRadius * ((scaleX + scaleY) / 2), color);
    }
    break;

    case HITBOX_POLY:
    {
        for (int i = 0; i < h->polygonHitbox.count; i++)
        {
            Vector2 p1 = {
                centerPos.x + h->offset.x * scaleX + h->polygonHitbox.vertices[i].x * scaleX,
                centerPos.y + h->offset.y * scaleY + h->polygonHitbox.vertices[i].y * scaleY};
            Vector2 p2 = {
                centerPos.x + h->offset.x * scaleX + h->polygonHitbox.vertices[(i + 1) % h->polygonHitbox.count].x * scaleX,
                centerPos.y + h->offset.y * scaleY + h->polygonHitbox.vertices[(i + 1) % h->polygonHitbox.count].y * scaleY};
            DrawLineV(p1, p2, color);
        }
    }
    break;

    default:
        break;
    }
}

void DrawComponents(InterpreterContext *intp)
{
    ClearBackground(intp->backgroundColor);

    for (int i = 0; i < intp->componentCount; i++)
    {
        SceneComponent component = intp->components[i];
        if (!component.isVisible)
        {
            continue;
        }
        if (component.isSprite)
        {
            DrawTexturePro(
                component.sprite.texture,
                (Rectangle){0, 0, (float)component.sprite.texture.width, (float)component.sprite.texture.height},
                (Rectangle){
                    component.sprite.position.x - intp->cameraOffset.x,
                    component.sprite.position.y - intp->cameraOffset.y,
                    (float)component.sprite.width,
                    (float)component.sprite.height},
                (Vector2){component.sprite.width / 2.0f, component.sprite.height / 2.0f},
                component.sprite.rotation,
                WHITE);
            if (intp->shouldShowHitboxes)
            {
                DrawHitbox(
                    &component.sprite.hitbox,
                    Vector2Subtract(component.sprite.position, intp->cameraOffset),
                    (Vector2){component.sprite.width, component.sprite.height},
                    (Vector2){component.sprite.texture.width, component.sprite.texture.height},
                    RED);
            }
            continue;
        }
        else
        {
            switch (component.prop.propType)
            {
            case PROP_TEXTURE:
                break; //
            case PROP_RECTANGLE:
                DrawRectangle(component.prop.position.x - intp->cameraOffset.x, component.prop.position.y - intp->cameraOffset.y, component.prop.width, component.prop.height, component.prop.color);
                break;
            case PROP_CIRCLE:
                DrawCircle(component.prop.position.x - intp->cameraOffset.x, component.prop.position.y - intp->cameraOffset.y, component.prop.width / 2, component.prop.color);
                break;
            default:
                AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of bounds enum{O201}"}, LOG_LEVEL_ERROR);
            }
            if (intp->shouldShowHitboxes)
            {
                DrawHitbox(
                    &component.prop.hitbox,
                    Vector2Subtract(component.prop.position, intp->cameraOffset),
                    (Vector2){component.prop.width, component.prop.height},
                    (Vector2){component.prop.width, component.prop.height},
                    RED);
            }
        }
    }
}

bool CheckCollisionPolyPoly(Polygon *a, Vector2 aPos, Vector2 aSize, Vector2 aTexSize, Polygon *b, Vector2 bPos, Vector2 bSize, Vector2 bTexSize)
{
    float scaleAX = aSize.x / aTexSize.x;
    float scaleAY = aSize.y / aTexSize.y;
    float scaleBX = bSize.x / bTexSize.x;
    float scaleBY = bSize.y / bTexSize.y;

    for (int i = 0; i < a->count; i++)
    {
        Vector2 a1 = {aPos.x + a->vertices[i].x * scaleAX,
                      aPos.y + a->vertices[i].y * scaleAY};
        Vector2 a2 = {aPos.x + a->vertices[(i + 1) % a->count].x * scaleAX,
                      aPos.y + a->vertices[(i + 1) % a->count].y * scaleAY};

        for (int j = 0; j < b->count; j++)
        {
            Vector2 b1 = {bPos.x + b->vertices[j].x * scaleBX,
                          bPos.y + b->vertices[j].y * scaleBY};
            Vector2 b2 = {bPos.x + b->vertices[(j + 1) % b->count].x * scaleBX,
                          bPos.y + b->vertices[(j + 1) % b->count].y * scaleBY};

            if (CheckCollisionLines(a1, a2, b1, b2, NULL))
            {
                return true;
            }
        }
    }
    return false;
}

bool CheckCollisionPolyCircle(Hitbox *h, Vector2 centerPos, Vector2 spriteSize, Vector2 texSize,
                              Vector2 circlePos, float circleRadius)
{
    float scaleX = spriteSize.x / texSize.x;
    float scaleY = spriteSize.y / texSize.y;

    Vector2 transformed[64];
    for (int i = 0; i < h->polygonHitbox.count; i++)
    {
        transformed[i].x = centerPos.x + h->offset.x * scaleX + h->polygonHitbox.vertices[i].x * scaleX;
        transformed[i].y = centerPos.y + h->offset.y * scaleY + h->polygonHitbox.vertices[i].y * scaleY;
    }

    if (CheckCollisionPointPoly(circlePos, transformed, h->polygonHitbox.count))
    {
        return true;
    }

    for (int i = 0; i < h->polygonHitbox.count; i++)
    {
        if (CheckCollisionPointCircle(transformed[i], circlePos, circleRadius))
        {
            return true;
        }
    }

    for (int i = 0; i < h->polygonHitbox.count; i++)
    {
        Vector2 a = transformed[i];
        Vector2 b = transformed[(i + 1) % h->polygonHitbox.count];
        if (CheckCollisionCircleLine(circlePos, circleRadius, a, b))
        {
            return true;
        }
    }

    return false;
}

bool CheckCollisionPolyRect(Polygon *poly, Vector2 polyPos, Vector2 polySize, Vector2 polyTexSize, Vector2 rectPos, Vector2 rectSize)
{
    float scaleX = polySize.x / polyTexSize.x;
    float scaleY = polySize.y / polyTexSize.y;
    Rectangle rect = {rectPos.x, rectPos.y, rectSize.x, rectSize.y};

    for (int i = 0; i < poly->count; i++)
    {
        Vector2 p = {
            polyPos.x + poly->vertices[i].x * scaleX,
            polyPos.y + poly->vertices[i].y * scaleY};
        if (CheckCollisionPointRec(p, rect))
            return true;
    }

    Vector2 corners[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height}};

    Vector2 transformed[64];
    for (int i = 0; i < poly->count; i++)
    {
        transformed[i].x = polyPos.x + poly->vertices[i].x * scaleX;
        transformed[i].y = polyPos.y + poly->vertices[i].y * scaleY;
    }

    for (int i = 0; i < 4; i++)
    {
        if (CheckCollisionPointPoly(corners[i], transformed, poly->count))
            return true;
    }

    for (int i = 0; i < poly->count; i++)
    {
        Vector2 a = transformed[i];
        Vector2 b = transformed[(i + 1) % poly->count];

        Vector2 r[4][2] = {
            {{rect.x, rect.y}, {rect.x + rect.width, rect.y}},
            {{rect.x + rect.width, rect.y}, {rect.x + rect.width, rect.y + rect.height}},
            {{rect.x + rect.width, rect.y + rect.height}, {rect.x, rect.y + rect.height}},
            {{rect.x, rect.y + rect.height}, {rect.x, rect.y}}};

        for (int j = 0; j < 4; j++)
        {
            if (CheckCollisionLines(a, b, r[j][0], r[j][1], NULL))
                return true;
        }
    }

    return false;
}

CollisionResult CheckCollisions(InterpreterContext *intp, int index)
{
    if (index < 0 || index >= intp->componentCount)
    {
        return COLLISION_RESULT_NONE;
    }

    SceneComponent *a = &intp->components[index];
    int layerA = a->isSprite ? a->sprite.layer : a->prop.layer;

    Hitbox *hitA = a->isSprite ? &a->sprite.hitbox : &a->prop.hitbox;
    Vector2 posA = a->isSprite ? a->sprite.position : a->prop.position;
    Vector2 sizeA = a->isSprite ? (Vector2){a->sprite.width, a->sprite.height} : (Vector2){a->prop.width, a->prop.height};
    Vector2 texA = a->isSprite ? (Vector2){a->sprite.texture.width, a->sprite.texture.height} : (Vector2){a->prop.texture.width, a->prop.texture.height};

    Rectangle aSimplifiedHitbox = (Rectangle){posA.x - sizeA.x / 2, posA.y - sizeA.y / 2, sizeA.x, sizeA.y};

    if (hitA->type != HITBOX_POLY)
    {
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Invalid sprite hitbox{I106}"}, LOG_LEVEL_WARNING);
        return COLLISION_RESULT_NONE;
    }

    for (int j = 0; j < intp->componentCount; j++)
    {
        if (j == index)
        {
            continue;
        }

        SceneComponent *b = &intp->components[j];
        int layerB = b->isSprite ? b->sprite.layer : b->prop.layer;

        bool aBlocks = (layerA == COMPONENT_LAYER_BLOCKING || layerA == COMPONENT_LAYER_COLLISION_EVENTS_AND_BLOCKING);
        bool aEvents = (layerA == COMPONENT_LAYER_COLLISION_EVENTS || layerA == COMPONENT_LAYER_COLLISION_EVENTS_AND_BLOCKING);

        bool bBlocks = (layerB == COMPONENT_LAYER_BLOCKING || layerB == COMPONENT_LAYER_COLLISION_EVENTS_AND_BLOCKING);
        bool bEvents = (layerB == COMPONENT_LAYER_COLLISION_EVENTS || layerB == COMPONENT_LAYER_COLLISION_EVENTS_AND_BLOCKING);

        if (!aBlocks && !aEvents && !bBlocks && !bEvents)
        {
            continue;
        }

        Hitbox *hitB = b->isSprite ? &b->sprite.hitbox : &b->prop.hitbox;
        Vector2 posB = b->isSprite ? b->sprite.position : b->prop.position;
        Vector2 sizeB = b->isSprite ? (Vector2){b->sprite.width, b->sprite.height} : (Vector2){b->prop.width, b->prop.height};
        Vector2 texB = b->isSprite ? (Vector2){b->sprite.texture.width, b->sprite.texture.height} : (Vector2){b->prop.texture.width, b->prop.texture.height};

        Rectangle bSimplifiedHitbox;

        switch (hitB->type)
        {
        case HITBOX_RECT:
            bSimplifiedHitbox = (Rectangle){posB.x, posB.y, sizeB.x, sizeB.y};
            break;
        case HITBOX_CIRCLE:
            bSimplifiedHitbox = (Rectangle){posB.x - sizeB.x / 2, posB.y - sizeB.y / 2, sizeB.x, sizeB.y};
            break;
        case HITBOX_POLY:
            bSimplifiedHitbox = (Rectangle){posB.x - sizeB.x / 2, posB.y - sizeB.y / 2, sizeB.x, sizeB.y};
            break;
        default:
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of bounds enum{O201}"}, LOG_LEVEL_WARNING);
            break;
        }

        if (!CheckCollisionRecs(aSimplifiedHitbox, bSimplifiedHitbox))
        {
            continue;
        }

        bool collided = false;

        switch (hitB->type)
        {
        case HITBOX_POLY:
            collided = CheckCollisionPolyPoly(&hitA->polygonHitbox, posA, sizeA, texA, &hitB->polygonHitbox, posB, sizeB, texB);
            break;
        case HITBOX_CIRCLE:
        {
            float scaleX = texB.x != 0 ? sizeB.x / texB.x : 1.0f;
            float scaleY = texB.y != 0 ? sizeB.y / texB.y : 1.0f;

            posB = (Vector2){
                posB.x + hitB->offset.x * scaleX,
                posB.y + hitB->offset.y * scaleY};

            collided = CheckCollisionPolyCircle(hitA, posA, sizeA, texA, posB, hitB->circleHitboxRadius * ((scaleX + scaleY) / 2));
            break;
        }
        case HITBOX_RECT:
            collided = CheckCollisionPolyRect(&hitA->polygonHitbox, posA, sizeA, texA, posB, hitB->rectHitboxSize);
            break;
        default:
            AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "Out of bounds enum{O201}"}, LOG_LEVEL_WARNING);
            break;
        }

        if (collided)
        {
            bool triggerEvent = aEvents || bEvents;
            bool triggerBlock = aBlocks && bBlocks;

            if (triggerEvent && triggerBlock)
            {
                return COLLISION_RESULT_EVENT_AND_BLOCKING;
            }
            if (triggerEvent)
            {
                return COLLISION_RESULT_EVENT;
            }
            if (triggerBlock)
            {
                return COLLISION_RESULT_BLOCKING;
            }
        }
    }

    return COLLISION_RESULT_NONE;
}

void HandleForces(InterpreterContext *intp)
{
    int i = 0;
    while (i < intp->forceCount)
    {
        Force *f = &intp->forces[i];
        Vector2 *pos = &intp->components[f->componentIndex].sprite.position;

        float speed = f->pixelsPerSecond;
        float angle = f->angle * (PI / 180.0f);
        float vx = cosf(angle) * speed;
        float vy = -sinf(angle) * speed;
        float deltaTime = GetFrameTime();

        Vector2 prevPos = *pos;

        pos->x += vx * deltaTime;
        pos->y += vy * deltaTime;

        f->duration -= deltaTime;

        CollisionResult result = CheckCollisions(intp, f->componentIndex);
        if (result == COLLISION_RESULT_BLOCKING || result == COLLISION_RESULT_EVENT_AND_BLOCKING)
        {
            *pos = prevPos;
        }

        if (f->duration <= 0)
        {
            for (int j = i; j < intp->forceCount - 1; j++)
            {
                intp->forces[j] = intp->forces[j + 1];
            }
            intp->forceCount--;
            continue;
        }

        i++;
    }
}

void HandleSounds(InterpreterContext *intp)
{
    float frameTime = GetFrameTime();
    for (int i = 0; i < intp->soundCount; i++)
    {
        intp->sounds[i].timeLeft -= frameTime;
        if (intp->sounds[i].timeLeft <= 0)
        {
            UnloadSound(intp->sounds[i].sound);
            for (int j = i; j < intp->soundCount - 1; j++)
            {
                intp->sounds[j] = intp->sounds[j + 1];
            }
            intp->soundCount--;
            i--;
        }
        if (intp->hasSoundOnChanged)
        {
            float volume = intp->isSoundOn ? 1.0f : 0.0f;
            SetSoundVolume(intp->sounds[i].sound, volume);
        }
    }
    if (intp->hasSoundOnChanged)
    {
        intp->hasSoundOnChanged = false;
    }
}

bool HandleGameScreen(InterpreterContext *intp, RuntimeGraphContext *graph, Vector2 mousePos, Rectangle screenBoundary)
{
    ClearBackground(BLACK);

    if (IsKeyPressed(KEY_P))
    {
        intp->isPaused = !intp->isPaused;
    }

    if (intp->isPaused)
    {
        DrawComponents(intp);
        DrawRectangleRec(screenBoundary, COLOR_INTP_PAUSE_BLUR);
        return true;
    }

    UpdateSpecialValues(intp, mousePos, screenBoundary);

    if (intp->isFirstFrame)
    {
        intp->onButtonNodeIndexes = malloc(sizeof(int) * graph->nodeCount);
        for (int i = 0; i < graph->nodeCount; i++)
        {
            switch (graph->nodes[i].type)
            {
            case NODE_EVENT_START:
                InterpretStringOfNodes(i, intp, graph, 0);
                break;
            case NODE_EVENT_TICK:
                if (intp->tickNodeIndexesCount < MAX_TICK_NODES)
                {
                    intp->tickNodeIndexes[intp->tickNodeIndexesCount] = i;
                    intp->tickNodeIndexesCount++;
                }
                break;
            case NODE_EVENT_ON_BUTTON:
                intp->onButtonNodeIndexes[intp->onButtonNodeIndexesCount++] = i;
                break;
            default:
                break;
            }
        }
        intp->onButtonNodeIndexes = realloc(intp->onButtonNodeIndexes, sizeof(int) * intp->onButtonNodeIndexesCount);

        intp->isFirstFrame = false;
    }
    else
    {
        intp->newLogMessage = false;
    }

    for (int i = 0; i < intp->onButtonNodeIndexesCount; i++)
    {
        int nodeIndex = intp->onButtonNodeIndexes[i];
        KeyboardKey key = graph->nodes[nodeIndex].inputPins[0]->pickedOption;
        KeyAction action = graph->nodes[nodeIndex].inputPins[1]->pickedOption;

        if (key == -1)
        {
            continue;
        }

        bool triggered = false;
        switch (action)
        {
        case KEY_ACTION_PRESSED:
            triggered = IsKeyPressed(key);
            break;
        case KEY_ACTION_RELEASED:
            triggered = IsKeyReleased(key);
            break;
        case KEY_ACTION_DOWN:
            triggered = IsKeyDown(key);
            break;
        case KEY_ACTION_NOT_DOWN:
            triggered = IsKeyUp(key);
            break;
        }

        if (triggered)
        {
            InterpretStringOfNodes(nodeIndex, intp, graph, 0);
        }
    }

    if (intp->tickNodeIndexesCount == 0)
    {
        AddToLogFromInterpreter(intp, (Value){.type = VAL_STRING, .string = "No tick node found{I211}"}, LOG_LEVEL_ERROR);
        return false;
    }
    else
    {
        for (int i = 0; i < intp->tickNodeIndexesCount; i++)
        {
            InterpretStringOfNodes(intp->tickNodeIndexes[i], intp, graph, 0);
        }
    }

    HandleForces(intp);

    HandleSounds(intp);

    DrawComponents(intp);

    for (int i = 0; i < intp->valueCount; i++)
    {
        if (intp->values[i].type == VAL_SPRITE)
        {
            intp->components[intp->values[i].componentIndex].sprite.isVisible = intp->components[intp->values[i].componentIndex].isVisible;
            intp->values[i].sprite = intp->components[intp->values[i].componentIndex].sprite;
        }
    }

    static Vector2 lastShake = {0, 0};
    if (intp->shakeCameraTimeRemaining > 0.0f)
    {
        intp->shakeCameraTimeRemaining -= GetFrameTime();

        intp->cameraOffset.x -= lastShake.x;
        intp->cameraOffset.y -= lastShake.y;

        lastShake.x = GetRandomValue(-intp->shakeCameraIntensity, intp->shakeCameraIntensity);
        lastShake.y = GetRandomValue(-intp->shakeCameraIntensity, intp->shakeCameraIntensity);

        intp->cameraOffset.x += lastShake.x;
        intp->cameraOffset.y += lastShake.y;
    }
    else if (lastShake.x != 0 || lastShake.y != 0)
    {
        intp->cameraOffset.x -= lastShake.x;
        intp->cameraOffset.y -= lastShake.y;

        lastShake.x = 0;
        lastShake.y = 0;
    }

    return true;
}