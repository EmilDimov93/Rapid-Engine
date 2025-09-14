#include "Nodes.h"

GraphContext InitGraphContext()
{
    GraphContext graph;

    graph.nodes = NULL;
    graph.nodeCount = 0;
    graph.nextNodeID = 1;

    graph.pins = NULL;
    graph.pinCount = 0;
    graph.nextPinID = 1;

    graph.links = NULL;
    graph.linkCount = 0;
    graph.nextLinkID = 1;

    return graph;
}

void FreeGraphContext(GraphContext *graph)
{
    if (!graph)
        return;

    if (graph->pins)
    {
        free(graph->pins);
        graph->pins = NULL;
    }

    if (graph->links)
    {
        free(graph->links);
        graph->links = NULL;
    }

    if (graph->nodes)
    {
        free(graph->nodes);
        graph->nodes = NULL;
    }

    graph->nodeCount = 0;
    graph->nextNodeID = 0;
    graph->pinCount = 0;
    graph->nextPinID = 0;
    graph->linkCount = 0;
    graph->nextLinkID = 0;
}

int FindPinIndexByID(GraphContext *graph, int id)
{
    for (int i = 0; i < graph->pinCount; i++)
    {
        if (graph->pins[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

int SaveGraphToFile(const char *filename, GraphContext *graph)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        return 1;
    }

    fwrite(&graph->nextNodeID, sizeof(int), 1, file);
    fwrite(&graph->nextPinID, sizeof(int), 1, file);
    fwrite(&graph->nextLinkID, sizeof(int), 1, file);

    fwrite(&graph->nodeCount, sizeof(int), 1, file);
    fwrite(graph->nodes, sizeof(Node), graph->nodeCount, file);

    fwrite(&graph->pinCount, sizeof(int), 1, file);
    fwrite(graph->pins, sizeof(Pin), graph->pinCount, file);

    fwrite(&graph->linkCount, sizeof(int), 1, file);
    fwrite(graph->links, sizeof(Link), graph->linkCount, file);

    fclose(file);

    return 0;
}

bool LoadGraphFromFile(const char *filename, GraphContext *graph)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        return false;
    }

    fread(&graph->nextNodeID, sizeof(int), 1, file);
    fread(&graph->nextPinID, sizeof(int), 1, file);
    fread(&graph->nextLinkID, sizeof(int), 1, file);

    fread(&graph->nodeCount, sizeof(int), 1, file);
    graph->nodes = malloc(sizeof(Node) * graph->nodeCount);
    fread(graph->nodes, sizeof(Node), graph->nodeCount, file);

    fread(&graph->pinCount, sizeof(int), 1, file);
    graph->pins = malloc(sizeof(Pin) * graph->pinCount);
    fread(graph->pins, sizeof(Pin), graph->pinCount, file);

    fread(&graph->linkCount, sizeof(int), 1, file);
    graph->links = malloc(sizeof(Link) * graph->linkCount);
    fread(graph->links, sizeof(Link), graph->linkCount, file);

    fclose(file);

    graph->variables = NULL;

    graph->variables = malloc(sizeof(char *) * 1);
    graph->variableTypes = malloc(sizeof(NodeType) * 1);
    graph->variables[0] = strmac(NULL, 5, "NONE");
    graph->variableTypes[0] = NODE_UNKNOWN;
    graph->variablesCount = 1;

    for (int i = 1; i < graph->nodeCount; i++)
    {
        if (graph->nodes[i].type == NODE_CREATE_NUMBER || graph->nodes[i].type == NODE_CREATE_STRING || graph->nodes[i].type == NODE_CREATE_BOOL || graph->nodes[i].type == NODE_CREATE_COLOR || graph->nodes[i].type == NODE_CREATE_SPRITE)
        {
            graph->variables = realloc(graph->variables, sizeof(char *) * (graph->variablesCount + 1));
            graph->variables[graph->variablesCount] = strmac(NULL, MAX_VARIABLE_NAME_SIZE, graph->nodes[i].name);

            graph->variableTypes = realloc(graph->variableTypes, sizeof(int) * (graph->variablesCount + 1));
            graph->variableTypes[graph->variablesCount] = graph->nodes[i].type;

            graph->variablesCount++;
        }
    }

    return true;
}

Pin CreatePin(GraphContext *graph, int nodeID, bool isInput, PinType type, int index, Vector2 pos)
{
    Pin pin = {0};
    pin.id = graph->nextPinID++;
    pin.nodeID = nodeID;
    pin.isInput = isInput;
    pin.type = type;
    pin.posInNode = index;
    pin.position = pos;
    pin.pickedOption = 0;
    switch (type)
    {
    case PIN_FIELD_NUM:
        strmac(pin.textFieldValue, 2, "0");
        break;
    case PIN_FIELD_BOOL:
        strmac(pin.textFieldValue, 6, "false");
        break;
    case PIN_FIELD_COLOR:
        strmac(pin.textFieldValue, 9, "00000000");
        break;
    case PIN_FIELD_KEY:
        pin.pickedOption = -1;
        break;
    default:
        break;
    }
    return pin;
}

char *AssignAvailableVarName(GraphContext *graph, const char *baseName) {
    char temp[MAX_VARIABLE_NAME_SIZE];
    int suffix = 1;
    bool exists;

    do {
        strmac(temp, MAX_VARIABLE_NAME_SIZE, "%s %d", baseName, suffix);
        exists = false;
        for (int i = 0; i < graph->nodeCount; i++) {
            if (strcmp(temp, graph->nodes[i].name) == 0) {
                exists = true;
                suffix++;
                break;
            }
        }
    } while (exists);

    char *name = malloc(strlen(temp) + 1);
    strmac(name, MAX_VARIABLE_NAME_SIZE, "%s", temp);
    return name;
}

bool CreateNode(GraphContext *graph, NodeType type, Vector2 pos)
{
    Node node = {0};
    node.id = graph->nextNodeID++;
    node.type = type;
    node.position = pos;

    switch (node.type)
    {
    case NODE_CREATE_NUMBER:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "%s", AssignAvailableVarName(graph, "Number"));
        break;
    case NODE_CREATE_STRING:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "%s", AssignAvailableVarName(graph, "String"));
        break;
    case NODE_CREATE_BOOL:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "%s", AssignAvailableVarName(graph, "Boolean"));
        break;
    case NODE_CREATE_COLOR:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "%s", AssignAvailableVarName(graph, "Color"));
        break;
    case NODE_CREATE_SPRITE:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "%s", AssignAvailableVarName(graph, "Sprite"));
        break;
    case NODE_GET_VARIABLE:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "");
        break;
    case NODE_SET_VARIABLE:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE, "");
        break;
    default:
        strmac(node.name, MAX_VARIABLE_NAME_SIZE,  "Node %d", node.id);
    }

    if (type == NODE_UNKNOWN)
    {
        return false;
    }

    int inputCount = getNodeInfoByType(type, INPUT_COUNT);
    int outputCount = getNodeInfoByType(type, OUTPUT_COUNT);

    if (inputCount > MAX_NODE_PINS || outputCount > MAX_NODE_PINS)
    {
        return false;
    }

    int newPinCapacity = graph->pinCount + inputCount + outputCount;
    Pin *newPins = realloc(graph->pins, sizeof(Pin) * newPinCapacity);
    if (!newPins && newPinCapacity != 0)
    {
        return false;
    }
    graph->pins = newPins;

    for (int i = 0; i < inputCount; i++)
    {
        Pin pin = CreatePin(graph, node.id, true, getInputsByType(type)[i], i, (Vector2){0, 0});
        graph->pins[graph->pinCount] = pin;
        node.inputPins[node.inputCount++] = pin.id;
        graph->pinCount++;
    }

    for (int i = 0; i < outputCount; i++)
    {
        Pin pin = CreatePin(graph, node.id, false, getOutputsByType(type)[i], i, (Vector2){0, 0});
        graph->pins[graph->pinCount] = pin;
        node.outputPins[node.outputCount++] = pin.id;
        graph->pinCount++;
    }

    Node *newNodes = realloc(graph->nodes, sizeof(Node) * (graph->nodeCount + 1));
    if (!newNodes)
    {
        return false;
    }
    graph->nodes = newNodes;
    graph->nodes[graph->nodeCount++] = node;

    return true;
}

int GetPinIndexByID(int id, GraphContext *graph)
{
    for (int i = 0; i < graph->pinCount; i++)
    {
        if (graph->pins[i].id == id)
            return i;
    }
    return -1;
}

bool DuplicateNode(GraphContext *graph, const Node *src, Vector2 pos)
{
    Node node = {0};
    node.id = graph->nextNodeID++;
    node.type = src->type;
    node.position = pos;
    strmac(node.name, MAX_VARIABLE_NAME_SIZE, "%s", src->name);

    int inputCount  = src->inputCount;
    int outputCount = src->outputCount;

    int newPinCapacity = graph->pinCount + inputCount + outputCount;
    Pin *newPins = realloc(graph->pins, sizeof(Pin) * newPinCapacity);
    if (!newPins && newPinCapacity != 0)
    {
        return false;
    }
    graph->pins = newPins;

    for (int i = 0; i < inputCount; ++i)
    {
        int srcPinID = src->inputPins[i];
        int srcIdx = GetPinIndexByID(srcPinID, graph);
        if (srcIdx < 0) { return false; }

        Pin *srcPin = &graph->pins[srcIdx];
        Vector2 offset = { srcPin->position.x - src->position.x, srcPin->position.y - src->position.y };
        Vector2 newPinPos = { pos.x + offset.x, pos.y + offset.y };

        Pin pin = CreatePin(graph, node.id, true, srcPin->type, srcPin->posInNode, newPinPos);

        if(srcPin->type == PIN_FIELD_NUM || srcPin->type == PIN_FIELD_STRING || srcPin->type == PIN_FIELD_BOOL || srcPin->type == PIN_FIELD_COLOR || srcPin->type == PIN_FIELD_KEY){
            strmac(pin.textFieldValue, MAX_LITERAL_NODE_FIELD_SIZE, "%s", srcPin->textFieldValue);
        }
        else if(srcPin->type == PIN_DROPDOWN_COMPARISON_OPERATOR || srcPin->type == PIN_DROPDOWN_GATE || srcPin->type == PIN_DROPDOWN_ARITHMETIC || srcPin->type == PIN_DROPDOWN_KEY_ACTION || srcPin->type == PIN_VARIABLE || srcPin->type == PIN_SPRITE_VARIABLE){
            pin.pickedOption = srcPin->pickedOption;
        }
        else if(srcPin->type == PIN_EDIT_HITBOX){
            pin.hitbox = srcPin->hitbox;
        }

        graph->pins[graph->pinCount] = pin;
        node.inputPins[node.inputCount++] = pin.id;
        graph->pinCount++;
    }

    for (int i = 0; i < outputCount; ++i)
    {
        int srcPinID = src->outputPins[i];
        int srcIdx = GetPinIndexByID(srcPinID, graph);
        if (srcIdx < 0) { return false; }

        Pin *srcPin = &graph->pins[srcIdx];
        Vector2 offset = { srcPin->position.x - src->position.x, srcPin->position.y - src->position.y };
        Vector2 newPinPos = { pos.x + offset.x, pos.y + offset.y };

        Pin pin = CreatePin(graph, node.id, false, srcPin->type, srcPin->posInNode, newPinPos);

        graph->pins[graph->pinCount] = pin;
        node.outputPins[node.outputCount++] = pin.id;
        graph->pinCount++;
    }

    Node *newNodes = realloc(graph->nodes, sizeof(Node) * (graph->nodeCount + 1));
    if (!newNodes)
    {
        return false;
    }
    graph->nodes = newNodes;
    graph->nodes[graph->nodeCount++] = node;

    return true;
}

void CreateLink(GraphContext *graph, Pin Pin1, Pin Pin2)
{
    if (Pin1.isInput == Pin2.isInput)
        return;
    if ((Pin1.type == PIN_FLOW && Pin2.type != PIN_FLOW) || (Pin1.type != PIN_FLOW && Pin2.type == PIN_FLOW))
        return;
    if (Pin1.nodeID == Pin2.nodeID)
        return;
    if (Pin1.type != Pin2.type && Pin1.type != PIN_ANY_VALUE && Pin2.type != PIN_ANY_VALUE)
        return;

    Link link = {0};

    if (Pin1.isInput)
    {
        link.inputPinID = Pin1.id;
        link.outputPinID = Pin2.id;
    }
    else
    {
        link.inputPinID = Pin2.id;
        link.outputPinID = Pin1.id;
    }

    int inputPinIndex = GetPinIndexByID(link.inputPinID, graph);
    int outputPinIndex = GetPinIndexByID(link.outputPinID, graph);

    Pin inputPin = graph->pins[inputPinIndex];
    Pin outputPin = graph->pins[outputPinIndex];

    for (int i = 0; i < graph->linkCount; i++)
    {
        Link l = graph->links[i];

        bool remove = false;

        if (l.inputPinID == inputPin.id && inputPin.type != PIN_FLOW)
            remove = true;
        if (l.outputPinID == outputPin.id && outputPin.type == PIN_FLOW)
            remove = true;

        if (remove)
        {
            for (int j = i; j < graph->linkCount - 1; j++)
            {
                graph->links[j] = graph->links[j + 1];
            }
            graph->linkCount--;
            i--;
        }
    }

    graph->links = realloc(graph->links, sizeof(Link) * (graph->linkCount + 1));
    graph->links[graph->linkCount++] = link;
}

void DeleteNode(GraphContext *graph, int nodeID)
{
    if (graph->nodeCount == 0)
        return;

    int nodeIndex = -1;
    for (int i = 0; i < graph->nodeCount; i++)
    {
        if (graph->nodes[i].id == nodeID)
        {
            nodeIndex = i;
            break;
        }
    }
    if (nodeIndex == -1)
        return;

    if (graph->nodes[nodeIndex].type == NODE_CREATE_NUMBER || graph->nodes[nodeIndex].type == NODE_CREATE_STRING || graph->nodes[nodeIndex].type == NODE_CREATE_BOOL || graph->nodes[nodeIndex].type == NODE_CREATE_COLOR || graph->nodes[nodeIndex].type == NODE_CREATE_SPRITE)
    {
        int variableToDeleteIndex = -1;
        for (int i = 0; i < graph->variablesCount; i++)
        {
            if (strcmp(graph->nodes[nodeIndex].name, graph->variables[i]) == 0)
            {
                variableToDeleteIndex = i;
                break;
            }
        }

        if (variableToDeleteIndex == -1)
        {
            return;
        }

        free(graph->variables[variableToDeleteIndex]);

        for (int i = 0; i < graph->nodeCount; i++)
        {
            if (graph->nodes[i].type == NODE_GET_VARIABLE || graph->nodes[i].type == NODE_SET_VARIABLE)
            {
                for (int j = 0; j < graph->pinCount; j++)
                {
                    if (graph->pins[j].id == graph->nodes[i].inputPins[graph->nodes[i].type == NODE_GET_VARIABLE ? 0 : 1])
                    {
                        if (graph->pins[j].pickedOption > variableToDeleteIndex)
                        {
                            graph->pins[j].pickedOption--;
                        }
                        else if(graph->pins[j].pickedOption == variableToDeleteIndex){
                            graph->pins[j].pickedOption = 0;
                        }
                    }
                }
            }
        }

        for (int i = variableToDeleteIndex; i < graph->variablesCount - 1; i++)
        {
            graph->variables[i] = graph->variables[i + 1];
            graph->variableTypes[i] = graph->variableTypes[i + 1];
        }

        graph->variablesCount--;

        if (graph->variablesCount == 0)
        {
            free(graph->variables);
            graph->variables = NULL;
        }
        else
        {
            char **resized = realloc(graph->variables, graph->variablesCount * sizeof(char *));
            if (resized)
                graph->variables = resized;
        }
        graph->variableTypes = realloc(graph->variableTypes, graph->variablesCount * sizeof(NodeType));
    }

    graph->nodes[nodeIndex] = graph->nodes[graph->nodeCount - 1];
    graph->nodeCount--;

    if (graph->nodeCount == 0)
    {
        free(graph->nodes);
        graph->nodes = NULL;
    }
    else
    {
        Node *resized = realloc(graph->nodes, graph->nodeCount * sizeof(Node));
        if (resized)
            graph->nodes = resized;
    }

    int *pinsToDelete = malloc(graph->pinCount * sizeof(int));
    int pinsToDeleteCount = 0;

    for (int i = 0; i < graph->pinCount;)
    {
        if (graph->pins[i].nodeID == nodeID)
        {
            pinsToDelete[pinsToDeleteCount++] = graph->pins[i].id;
            graph->pins[i] = graph->pins[graph->pinCount - 1];
            graph->pinCount--;

            if (graph->pinCount == 0)
            {
                free(graph->pins);
                graph->pins = NULL;
                break;
            }
            else
            {
                Pin *resized = realloc(graph->pins, graph->pinCount * sizeof(Pin));
                if (resized)
                    graph->pins = resized;
            }
        }
        else
        {
            i++;
        }
    }

    for (int i = 0; i < graph->linkCount;)
    {
        bool remove = false;
        for (int j = 0; j < pinsToDeleteCount; j++)
        {
            if (graph->links[i].inputPinID == pinsToDelete[j] ||
                graph->links[i].outputPinID == pinsToDelete[j])
            {
                remove = true;
                break;
            }
        }

        if (remove)
        {
            graph->links[i] = graph->links[graph->linkCount - 1];
            graph->linkCount--;

            if (graph->linkCount == 0)
            {
                free(graph->links);
                graph->links = NULL;
                break;
            }
            else
            {
                Link *resized = realloc(graph->links, graph->linkCount * sizeof(Link));
                if (resized)
                    graph->links = resized;
            }
        }
        else
        {
            i++;
        }
    }

    free(pinsToDelete);
}

void RemoveConnections(GraphContext *graph, int pinID)
{
    for (int i = 0; i < graph->linkCount;)
    {
        if (graph->links[i].outputPinID == pinID || graph->links[i].inputPinID == pinID)
        {
            graph->links[i] = graph->links[graph->linkCount - 1];
            graph->linkCount--;
            continue;
        }
        i++;
    }

    graph->links = realloc(graph->links, sizeof(Link) * graph->linkCount);
}