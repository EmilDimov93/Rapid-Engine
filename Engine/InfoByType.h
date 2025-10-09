#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"

#define typesCount (sizeof(NodeInfoByType) / sizeof(NodeInfoByType[0]))
#define dropdownTypesCount (sizeof(PinDropdownOptionsByType) / sizeof(PinDropdownOptionsByType[0]))

typedef enum
{
    NODE_UNKNOWN = 0,

    NODE_CREATE_NUMBER = 100,
    NODE_CREATE_STRING = 101,
    NODE_CREATE_BOOL = 102,
    NODE_CREATE_COLOR = 103,

    NODE_EVENT_START = 200,
    NODE_EVENT_TICK = 201,
    NODE_EVENT_ON_BUTTON = 202,
    NODE_CREATE_CUSTOM_EVENT = 203,
    NODE_CALL_CUSTOM_EVENT = 204,

    NODE_GET_VARIABLE = 300,
    NODE_GET_SCREEN_WIDTH = 301,
    NODE_GET_SCREEN_HEIGHT = 302,
    NODE_GET_MOUSE_POSITION = 303,
    NODE_GET_RANDOM_NUMBER = 304,

    NODE_SET_VARIABLE = 400,
    NODE_SET_BACKGROUND = 401,
    NODE_SET_FPS = 402,

    NODE_BRANCH = 500,
    NODE_LOOP = 501,
    NODE_DELAY = 502,
    NODE_FLIP_FLOP = 503,
    NODE_BREAK = 504,
    NODE_RETURN = 505,

    NODE_CREATE_SPRITE = 600,
    NODE_SPAWN_SPRITE = 601,
    NODE_DESTROY_SPRITE = 602,
    NODE_SET_SPRITE_POSITION = 603,
    NODE_SET_SPRITE_ROTATION = 604,
    NODE_SET_SPRITE_TEXTURE = 605,
    NODE_SET_SPRITE_SIZE = 606,
    NODE_MOVE_TO_SPRITE = 607,
    NODE_FORCE_SPRITE = 608,
    NODE_STOP_MOVEMENT_SPRITE = 609, //

    NODE_DRAW_PROP_TEXTURE = 700,
    NODE_DRAW_PROP_RECTANGLE = 701,
    NODE_DRAW_PROP_CIRCLE = 702,

    NODE_COMPARISON = 800,
    NODE_GATE = 801,
    NODE_ARITHMETIC = 802,

    NODE_PRINT_TO_LOG = 900,
    NODE_DRAW_DEBUG_LINE = 901,

    NODE_LITERAL_NUMBER = 1000,
    NODE_LITERAL_STRING = 1001,
    NODE_LITERAL_BOOL = 1002,
    NODE_LITERAL_COLOR = 1003,

    NODE_MOVE_CAMERA = 1100,
    NODE_ZOOM_CAMERA = 1101,
    NODE_GET_CAMERA_CENTER = 1102,

    NODE_PLAY_SOUND = 1200
} NodeType;

typedef enum
{
    PIN_NONE,
    PIN_FLOW,
    PIN_NUM,
    PIN_STRING,
    PIN_BOOL,
    PIN_COLOR,
    PIN_SPRITE,
    PIN_FIELD_NUM,
    PIN_FIELD_STRING,
    PIN_FIELD_BOOL,
    PIN_FIELD_COLOR,
    PIN_FIELD_KEY,
    PIN_DROPDOWN_COMPARISON_OPERATOR,
    PIN_DROPDOWN_GATE,
    PIN_DROPDOWN_ARITHMETIC,
    PIN_DROPDOWN_KEY_ACTION,
    PIN_VARIABLE,
    PIN_SPRITE_VARIABLE,
    PIN_ANY_VALUE,
    PIN_UNKNOWN_VALUE,
    PIN_EDIT_HITBOX,
    PIN_DROPDOWN_LAYER
} PinType;

typedef enum
{
    EQUAL_TO,
    GREATER_THAN,
    LESS_THAN
} Comparison;

typedef enum
{
    AND,
    OR,
    NOT,
    XOR,
    NAND,
    NOR
} Gate;

typedef enum
{
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO
} Arithmetic;

typedef enum
{
    KEY_ACTION_PRESSED,
    KEY_ACTION_RELEASED,
    KEY_ACTION_DOWN,
    KEY_ACTION_NOT_DOWN
} KeyAction;

typedef struct InfoByType
{
    NodeType type;

    int inputCount;
    int outputCount;

    int width;
    int height;

    Color color;

    bool isEditable;

    PinType inputs[16];
    PinType outputs[16];

    char *inputNames[16];
    char *outputNames[16];

    bool isNotImplemented;
} InfoByType;

static InfoByType NodeInfoByType[] = {
    {NODE_UNKNOWN, 0, 0, 50, 50},

    {NODE_CREATE_NUMBER, 2, 2, 140, 100, {100, 60, 120, 200}, true, {PIN_FLOW, PIN_NUM}, {PIN_FLOW, PIN_NUM}, {"Prev", "Set value"}, {"Next", "Get value"}},
    {NODE_CREATE_STRING, 2, 2, 120, 100, {100, 60, 120, 200}, true, {PIN_FLOW, PIN_STRING}, {PIN_FLOW, PIN_STRING}, {"Prev", "Set value"}, {"Next", "Get value"}},
    {NODE_CREATE_BOOL, 2, 2, 120, 100, {100, 60, 120, 200}, true, {PIN_FLOW, PIN_BOOL}, {PIN_FLOW, PIN_BOOL}, {"Prev", "Set value"}, {"Next", "Get value"}},
    {NODE_CREATE_COLOR, 2, 2, 120, 100, {100, 60, 120, 200}, true, {PIN_FLOW, PIN_COLOR}, {PIN_FLOW, PIN_COLOR}, {"Prev", "Set value"}, {"Next", "Get value"}},

    {NODE_EVENT_START, 0, 1, 150, 120, {148, 0, 0, 200}, false, {0}, {PIN_FLOW}, {0}, {"Next"}},
    {NODE_EVENT_TICK, 0, 1, 150, 120, {148, 0, 0, 200}, false, {0}, {PIN_FLOW}, {0}, {"Next"}},
    {NODE_EVENT_ON_BUTTON, 2, 1, 160, 120, {148, 0, 0, 200}, false, {PIN_FIELD_KEY, PIN_DROPDOWN_KEY_ACTION}, {PIN_FLOW}, {"Key", "Action"}, {"Next"}},
    {NODE_CREATE_CUSTOM_EVENT, 0, 1, 240, 200, {148, 0, 0, 200}, false, {0}, {PIN_FLOW}, {"Prev"}, {"Next"}, true},      // not implemented
    {NODE_CALL_CUSTOM_EVENT, 0, 1, 240, 200, {148, 0, 0, 200}, false, {PIN_FLOW}, {PIN_FLOW}, {"Prev"}, {"Next"}, true}, // not implemented

    {NODE_GET_VARIABLE, 1, 1, 140, 70, {60, 100, 159, 200}, false, {PIN_VARIABLE}, {PIN_UNKNOWN_VALUE}, {"Variable"}, {"Get value"}},
    {NODE_GET_SCREEN_WIDTH, 0, 1, 250, 70, {60, 100, 159, 200}, false, {0}, {PIN_NUM}, {0}, {"Screen Width"}},
    {NODE_GET_SCREEN_HEIGHT, 0, 1, 265, 70, {60, 100, 159, 200}, false, {0}, {PIN_NUM}, {0}, {"Screen Height"}},
    {NODE_GET_MOUSE_POSITION, 0, 2, 220, 95, {60, 100, 159, 200}, false, {0}, {PIN_NUM, PIN_NUM}, {0}, {"Mouse X", "Mouse Y"}},
    {NODE_GET_RANDOM_NUMBER, 0, 1, 260, 70, {60, 100, 159, 200}, false, {0}, {0}, {0}, {0}, true}, // not implemented

    {NODE_SET_VARIABLE, 3, 2, 140, 130, {60, 100, 159, 200}, false, {PIN_FLOW, PIN_VARIABLE, PIN_UNKNOWN_VALUE}, {PIN_FLOW, PIN_NONE}, {"Prev", "Variable", "Set value"}, {"Next", ""}}, // shouldn't have PIN_NONE
    {NODE_SET_BACKGROUND, 2, 1, 240, 100, {60, 100, 159, 200}, false, {PIN_FLOW, PIN_COLOR}, {PIN_FLOW}, {"Prev", "Color"}, {"Next"}},
    {NODE_SET_FPS, 2, 1, 140, 100, {60, 100, 159, 200}, false, {PIN_FLOW, PIN_NUM}, {PIN_FLOW}, {"Prev", "FPS"}, {"Next"}},

    {NODE_BRANCH, 2, 2, 130, 100, {90, 90, 90, 200}, false, {PIN_FLOW, PIN_BOOL}, {PIN_FLOW, PIN_FLOW}, {"Prev", "Condition"}, {"True", "False"}},
    {NODE_LOOP, 2, 2, 130, 100, {90, 90, 90, 200}, false, {PIN_FLOW, PIN_BOOL}, {PIN_FLOW, PIN_FLOW}, {"Prev", "Condition"}, {"Next", "Loop body"}},
    {NODE_DELAY, 2, 1, 130, 100, {90, 90, 90, 200}, false, {PIN_FLOW, PIN_NUM}, {PIN_FLOW}, {"Prev", "Seconds"}, {"Next"}, true},     // not implemented
    {NODE_FLIP_FLOP, 1, 2, 130, 100, {90, 90, 90, 200}, false, {PIN_FLOW}, {PIN_FLOW, PIN_FLOW}, {"Prev"}, {"Flip", "Flop"}, true},   // not implemented
    {NODE_BREAK, 1, 0, 130, 100, {90, 90, 90, 200}, false, {PIN_FLOW}, {0}, {"Prev"}, {0}, true},                                     // not implemented
    {NODE_RETURN, 2, 0, 130, 100, {90, 90, 90, 200}, false, {PIN_FLOW, PIN_UNKNOWN_VALUE}, {0}, {"Prev", "Return value"}, {0}, true}, // not implemented

    {NODE_CREATE_SPRITE, 6, 2, 220, 230, {70, 100, 70, 200}, true, {PIN_FLOW, PIN_STRING, PIN_NUM, PIN_NUM, PIN_DROPDOWN_LAYER, PIN_EDIT_HITBOX}, {PIN_FLOW, PIN_SPRITE}, {"Prev", "Texture file name", "Width", "Height", "Layer", "Hitbox"}, {"Next", "Sprite"}},
    {NODE_SPAWN_SPRITE, 5, 1, 120, 190, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE, PIN_NUM, PIN_NUM, PIN_NUM}, {PIN_FLOW}, {"Prev", "Sprite", "Pos X", "Pos Y", "Rotation"}, {"Next"}},
    {NODE_DESTROY_SPRITE, 2, 1, 120, 100, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE}, {PIN_FLOW}, {"Prev", "Sprite"}, {"Next"}},
    {NODE_SET_SPRITE_POSITION, 4, 1, 185, 160, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE, PIN_NUM, PIN_NUM}, {PIN_FLOW}, {"Prev", "Sprite", "X", "Y"}, {"Next"}},
    {NODE_SET_SPRITE_ROTATION, 3, 1, 180, 130, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE, PIN_NUM}, {PIN_FLOW}, {"Prev", "Sprite", "Rotation"}, {"Next"}},
    {NODE_SET_SPRITE_TEXTURE, 3, 1, 180, 130, {0, 0, 0, 255}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE, PIN_STRING}, {PIN_FLOW}, {"Prev", "Sprite", "Texture name"}, {"Next"}},
    {NODE_SET_SPRITE_SIZE, 4, 1, 170, 160, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE, PIN_NUM, PIN_NUM}, {PIN_FLOW}, {"Prev", "Sprite", "Width", "Height"}, {"Next"}},
    {NODE_MOVE_TO_SPRITE, 0, 0, 260, 36, {0, 0, 0, 255}, false, {0}, {0}, {0}, {0}, true}, // not implemented
    {NODE_FORCE_SPRITE, 5, 1, 160, 190, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_SPRITE_VARIABLE, PIN_NUM, PIN_NUM, PIN_NUM}, {PIN_FLOW}, {"Prev", "Sprite", "Pixels / second", "Angle", "Time"}, {"Next"}},

    {NODE_DRAW_PROP_TEXTURE, 0, 0, 260, 36, {0, 0, 0, 255}, false, {0}, {0}, {0}, {0}, true},                                                                                                                                                   // not implemented
    {NODE_DRAW_PROP_RECTANGLE, 7, 2, 230, 260, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_NUM, PIN_NUM, PIN_NUM, PIN_NUM, PIN_COLOR, PIN_DROPDOWN_LAYER}, {PIN_FLOW, PIN_NONE}, {"Prev", "Pos X", "Pos Y", "Width", "Height", "Color", "Layer"}, {"Next"}}, // shouldn't have PIN_NONE
    {NODE_DRAW_PROP_CIRCLE, 6, 2, 230, 240, {40, 110, 70, 200}, false, {PIN_FLOW, PIN_NUM, PIN_NUM, PIN_NUM, PIN_COLOR, PIN_DROPDOWN_LAYER}, {PIN_FLOW, PIN_NONE}, {"Prev", "Pos X", "Pos Y", "Radius", "Color", "Layer"}, {"Next"}},                      // shouldn't have PIN_NONE

    {NODE_COMPARISON, 4, 2, 210, 160, {60, 100, 159, 200}, false, {PIN_FLOW, PIN_DROPDOWN_COMPARISON_OPERATOR, PIN_NUM, PIN_NUM}, {PIN_FLOW, PIN_BOOL}, {"Prev", "Operator", "Value A", "Value B"}, {"Next", "Result"}},
    {NODE_GATE, 4, 2, 180, 160, {60, 100, 159, 200}, false, {PIN_FLOW, PIN_DROPDOWN_GATE, PIN_BOOL, PIN_BOOL}, {PIN_FLOW, PIN_BOOL}, {"Prev", "Gate", "Condition A", "Condition B"}, {"Next", "Result"}},
    {NODE_ARITHMETIC, 4, 2, 180, 160, {60, 100, 159, 200}, false, {PIN_FLOW, PIN_DROPDOWN_ARITHMETIC, PIN_NUM, PIN_NUM}, {PIN_FLOW, PIN_NUM}, {"Prev", "Arithmetic", "Number A", "Number B"}, {"Next", "Result"}},

    {NODE_PRINT_TO_LOG, 2, 1, 140, 100, {200, 170, 50, 200}, false, {PIN_FLOW, PIN_ANY_VALUE}, {PIN_FLOW}, {"Prev", "Print value"}, {"Next"}},
    {NODE_DRAW_DEBUG_LINE, 6, 1, 240, 220, {200, 170, 50, 200}, false, {PIN_FLOW, PIN_NUM, PIN_NUM, PIN_NUM, PIN_NUM, PIN_COLOR}, {PIN_FLOW}, {"Prev", "Start X", "Start Y", "End X", "End Y", "Color"}, {"Next"}},

    {NODE_LITERAL_NUMBER, 1, 1, 200, 70, {110, 85, 40, 200}, false, {PIN_FIELD_NUM}, {PIN_NUM}, {""}, {"number"}},
    {NODE_LITERAL_STRING, 1, 1, 200, 70, {110, 85, 40, 200}, false, {PIN_FIELD_STRING}, {PIN_STRING}, {""}, {"string"}},
    {NODE_LITERAL_BOOL, 1, 1, 180, 70, {110, 85, 40, 200}, false, {PIN_FIELD_BOOL}, {PIN_BOOL}, {""}, {"bool"}},
    {NODE_LITERAL_COLOR, 1, 1, 200, 70, {110, 85, 40, 200}, false, {PIN_FIELD_COLOR}, {PIN_COLOR}, {""}, {"color"}},

    {NODE_MOVE_CAMERA, 3, 1, 190, 130, {200, 130, 60, 200}, false, {PIN_FLOW, PIN_NUM, PIN_NUM}, {PIN_FLOW}, {"Prev", "Camera Delta X", "Camera Delta Y"}, {"Next"}},
    {NODE_ZOOM_CAMERA, 2, 1, 190, 100, {200, 130, 60, 200}, false, {PIN_FLOW, PIN_NUM}, {PIN_FLOW}, {"Prev", "Zoom Delta"}, {"Next"}},
    {NODE_GET_CAMERA_CENTER, 0, 2, 160, 100, {200, 130, 60, 200}, false, {0}, {PIN_NUM, PIN_NUM}, {0}, {"Center X", "Center Y"}},

    {NODE_PLAY_SOUND, 2, 1, 190, 100, {150, 255, 80, 200}, false, {PIN_FLOW, PIN_STRING}, {PIN_FLOW}, {"Prev", "Sound file name"}, {"Next"}}
};

static inline int NodeTypeToIndex(NodeType type)
{
    for (int i = 0; i < sizeof(NodeInfoByType) / sizeof(NodeInfoByType[0]); i++)
    {
        if (NodeInfoByType[i].type == type)
        {
            return i;
        }
    }
    return -1;
}

#define subMenuItemCount 10

const char *menuItems[] = {"Variable", "Event", "Get", "Set", "Flow", "Sprite", "Draw Prop", "Logical", "Debug", "Literal", "Camera", "Sound"};
const char *subMenuItems[][subMenuItemCount] = {
    {"Create number", "Create string", "Create bool", "Create color"},
    {"Event Start", "Event Tick", "Event On Button", "Create Custom Event", "Call Custom Event"},
    {"Get variable", "Get Screen Width", "Get Screen Height", "Get Mouse Positon", "Get Random Number"},
    {"Set variable", "Set Background", "Set FPS"},
    {"Branch", "Loop", "Delay", "Flip Flop", "Break", "Return"},
    {"Create sprite", "Spawn sprite", "Destroy sprite", "Set Sprite Position", "Set Sprite Rotation", "Set Sprite Texture", "Set Sprite Size", "Move To", "Force"},
    {"Draw Prop Texture", "Draw Prop Rectangle", "Draw Prop Circle"},
    {"Comparison", "Gate", "Arithmetic"},
    {"Print To Log", "Draw Debug Line"},
    {"Literal number", "Literal string", "Literal bool", "Literal color"},
    {"Move Camera", "Zoom Camera", "Get Camera Center"},
    {"Play Sound"}};

#define menuItemCount sizeof(menuItems) / sizeof(menuItems[0])

const int subMenuCounts[] = {4, 5, 5, 3, 6, 9, 3, 3, 2, 4, 3, 1};

typedef struct DropdownOptionsByPinType
{
    PinType type;
    int optionsCount;
    char **options;
    int boxWidth;
} DropdownOptionsByPinType;

static char *comparisonOps[] = {"Equal To", "Greater Than", "Less Than"};
static char *gateOps[] = {"AND", "OR", "NOT", "XOR", "NAND", "NOR"};
static char *arithmeticOps[] = {"ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "MODULO"};
static char *keyActionOps[] = {"Pressed", "Released", "Down", "Not down"};
static char *layerOps[] = {"No Collision", "Events only", "Block only", "Events & Block"};

static DropdownOptionsByPinType PinDropdownOptionsByType[] = {
    {PIN_DROPDOWN_COMPARISON_OPERATOR, 3, comparisonOps, 130},
    {PIN_DROPDOWN_GATE, 6, gateOps, 70},
    {PIN_DROPDOWN_ARITHMETIC, 5, arithmeticOps, 115},
    {PIN_DROPDOWN_KEY_ACTION, 4, keyActionOps, 110},
    {PIN_DROPDOWN_LAYER, 4, layerOps, 150}
};

static inline DropdownOptionsByPinType getPinDropdownOptionsByType(PinType type)
{
    for (int i = 0; i < dropdownTypesCount; i++)
    {
        if (type == PinDropdownOptionsByType[i].type)
        {
            return PinDropdownOptionsByType[i];
        }
    }

    return (DropdownOptionsByPinType){0};
}

typedef enum
{
    INFO_NODE_INPUT_COUNT,
    INFO_NODE_OUTPUT_COUNT,
    INFO_NODE_WIDTH,
    INFO_NODE_HEIGHT
} RequestedInfo;

static inline int getNodeInfoByType(NodeType type, RequestedInfo info)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return -1;
    }

    switch (info)
    {
    case INFO_NODE_INPUT_COUNT:
        return NodeInfoByType[index].inputCount;
    case INFO_NODE_OUTPUT_COUNT:
        return NodeInfoByType[index].outputCount;
    case INFO_NODE_WIDTH:
        return NodeInfoByType[index].width;
    case INFO_NODE_HEIGHT:
        return NodeInfoByType[index].height;
    default:
        return -1;
    }
}

static inline bool getIsEditableByType(NodeType type)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return false;
    }

    if (type == NodeInfoByType[index].type)
    {
        return NodeInfoByType[index].isEditable;
    }
}

static inline char **getNodeInputNamesByType(NodeType type)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return NULL;
    }

    if (type == NodeInfoByType[index].type)
    {
        return NodeInfoByType[index].inputNames;
    }
}

static inline char **getNodeOutputNamesByType(NodeType type)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return NULL;
    }

    if (type == NodeInfoByType[index].type)
    {
        return NodeInfoByType[index].outputNames;
    }
}

static inline Color getNodeColorByType(NodeType type)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return BLACK;
    }

    if (type == NodeInfoByType[index].type)
    {
        return NodeInfoByType[index].color;
    }
}

static inline PinType *getInputsByType(NodeType type)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return NULL;
    }

    if (NodeInfoByType[index].type == type)
    {
        return NodeInfoByType[index].inputs ? NodeInfoByType[index].inputs : NULL;
    }
    return NULL;
}

static inline PinType *getOutputsByType(NodeType type)
{
    int index = NodeTypeToIndex(type);
    if (index == -1)
    {
        return NULL;
    }

    if (NodeInfoByType[index].type == type)
    {
        return NodeInfoByType[index].outputs ? NodeInfoByType[index].outputs : NULL;
    }
}

static inline const char *NodeTypeToString(NodeType type)
{
    if (NodeInfoByType[NodeTypeToIndex(type)].isNotImplemented)
    {
        return "Not implemented";
    }

    switch (type)
    {
    case NODE_UNKNOWN:
        return "unknown";

    case NODE_CREATE_NUMBER:
        return "number";
    case NODE_CREATE_STRING:
        return "string";
    case NODE_CREATE_BOOL:
        return "bool";
    case NODE_CREATE_COLOR:
        return "color";

    case NODE_EVENT_START:
        return "Start";
    case NODE_EVENT_TICK:
        return "Tick";
    case NODE_EVENT_ON_BUTTON:
        return "On Button";
    case NODE_CREATE_CUSTOM_EVENT:
        return "Create Event";
    case NODE_CALL_CUSTOM_EVENT:
        return "Call Event";

    case NODE_GET_VARIABLE:
        return "Get var";
    case NODE_GET_SCREEN_WIDTH:
        return "Get screen width";
    case NODE_GET_SCREEN_HEIGHT:
        return "Get screen height";
    case NODE_GET_MOUSE_POSITION:
        return "Get mouse pos";
    case NODE_GET_RANDOM_NUMBER:
        return "Get random num";

    case NODE_SET_VARIABLE:
        return "Set var";
    case NODE_SET_BACKGROUND:
        return "Set Background";
    case NODE_SET_FPS:
        return "Set FPS";

    case NODE_BRANCH:
        return "Branch";
    case NODE_LOOP:
        return "Loop";
    case NODE_DELAY:
        return "Delay";
    case NODE_FLIP_FLOP:
        return "Flip Flop";
    case NODE_BREAK:
        return "Break";
    case NODE_RETURN:
        return "Return";

    case NODE_CREATE_SPRITE:
        return "Create sprite";
    case NODE_SET_SPRITE_POSITION:
        return "Set position";
    case NODE_SET_SPRITE_ROTATION:
        return "Set rotation";
    case NODE_SET_SPRITE_TEXTURE:
        return "Set texture";
    case NODE_SET_SPRITE_SIZE:
        return "Set size";
    case NODE_SPAWN_SPRITE:
        return "Spawn";
    case NODE_DESTROY_SPRITE:
        return "Destroy";
    case NODE_MOVE_TO_SPRITE:
        return "Move To";
    case NODE_FORCE_SPRITE:
        return "Force";

    case NODE_DRAW_PROP_TEXTURE:
        return "Prop Texture";
    case NODE_DRAW_PROP_RECTANGLE:
        return "Prop Rectangle";
    case NODE_DRAW_PROP_CIRCLE:
        return "Prop Circle";

    case NODE_COMPARISON:
        return "Comparison";
    case NODE_GATE:
        return "Gate";
    case NODE_ARITHMETIC:
        return "Arithmetic";

    case NODE_PRINT_TO_LOG:
        return "Print";
    case NODE_DRAW_DEBUG_LINE:
        return "Debug Line";

    case NODE_LITERAL_NUMBER:
        return "Literal num";
    case NODE_LITERAL_STRING:
        return "Literal string";
    case NODE_LITERAL_BOOL:
        return "Literal bool";
    case NODE_LITERAL_COLOR:
        return "Literal color";

    case NODE_MOVE_CAMERA:
        return "Move Camera";
    case NODE_ZOOM_CAMERA:
        return "Zoom Camera";
    case NODE_GET_CAMERA_CENTER:
        return "Get center";

    case NODE_PLAY_SOUND:
        return "Play Sound";

    default:
        return "invalid";
    }
}

static inline NodeType StringToNodeType(const char strType[])
{
    if (strcmp(strType, "unknown") == 0)
        return NODE_UNKNOWN;

    if (strcmp(strType, "Create number") == 0)
        return NODE_CREATE_NUMBER;
    if (strcmp(strType, "Create string") == 0)
        return NODE_CREATE_STRING;
    if (strcmp(strType, "Create bool") == 0)
        return NODE_CREATE_BOOL;
    if (strcmp(strType, "Create color") == 0)
        return NODE_CREATE_COLOR;

    if (strcmp(strType, "Event Start") == 0)
        return NODE_EVENT_START;
    if (strcmp(strType, "Event Tick") == 0)
        return NODE_EVENT_TICK;
    if (strcmp(strType, "Event On Button") == 0)
        return NODE_EVENT_ON_BUTTON;
    if (strcmp(strType, "Create Custom Event") == 0)
        return NODE_CREATE_CUSTOM_EVENT;
    if (strcmp(strType, "Call Custom Event") == 0)
        return NODE_CALL_CUSTOM_EVENT;

    if (strcmp(strType, "Get variable") == 0)
        return NODE_GET_VARIABLE;
    if (strcmp(strType, "Get Screen Width") == 0)
        return NODE_GET_SCREEN_WIDTH;
    if (strcmp(strType, "Get Screen Height") == 0)
        return NODE_GET_SCREEN_HEIGHT;
    if (strcmp(strType, "Get Mouse Positon") == 0)
        return NODE_GET_MOUSE_POSITION;
    if (strcmp(strType, "Get Random Number") == 0)
        return NODE_GET_RANDOM_NUMBER;

    if (strcmp(strType, "Set variable") == 0)
        return NODE_SET_VARIABLE;
    if (strcmp(strType, "Set Background") == 0)
        return NODE_SET_BACKGROUND;
    if (strcmp(strType, "Set FPS") == 0)
        return NODE_SET_FPS;

    if (strcmp(strType, "Branch") == 0)
        return NODE_BRANCH;
    if (strcmp(strType, "Loop") == 0)
        return NODE_LOOP;
    if (strcmp(strType, "Delay") == 0)
        return NODE_DELAY;
    if (strcmp(strType, "Flip Flop") == 0)
        return NODE_FLIP_FLOP;
    if (strcmp(strType, "Break") == 0)
        return NODE_BREAK;
    if (strcmp(strType, "Return") == 0)
        return NODE_RETURN;

    if (strcmp(strType, "Create sprite") == 0)
        return NODE_CREATE_SPRITE;
    if (strcmp(strType, "Set Sprite Position") == 0)
        return NODE_SET_SPRITE_POSITION;
    if (strcmp(strType, "Set Sprite Rotation") == 0)
        return NODE_SET_SPRITE_ROTATION;
    if (strcmp(strType, "Set Sprite Texture") == 0)
        return NODE_SET_SPRITE_TEXTURE;
    if (strcmp(strType, "Set Sprite Size") == 0)
        return NODE_SET_SPRITE_SIZE;
    if (strcmp(strType, "Spawn sprite") == 0)
        return NODE_SPAWN_SPRITE;
    if (strcmp(strType, "Destroy sprite") == 0)
        return NODE_DESTROY_SPRITE;
    if (strcmp(strType, "Move To") == 0)
        return NODE_MOVE_TO_SPRITE;
    if (strcmp(strType, "Force") == 0)
        return NODE_FORCE_SPRITE;

    if (strcmp(strType, "Draw Prop Texture") == 0)
        return NODE_DRAW_PROP_TEXTURE;
    if (strcmp(strType, "Draw Prop Rectangle") == 0)
        return NODE_DRAW_PROP_RECTANGLE;
    if (strcmp(strType, "Draw Prop Circle") == 0)
        return NODE_DRAW_PROP_CIRCLE;

    if (strcmp(strType, "Comparison") == 0)
        return NODE_COMPARISON;
    if (strcmp(strType, "Gate") == 0)
        return NODE_GATE;
    if (strcmp(strType, "Arithmetic") == 0)
        return NODE_ARITHMETIC;

    if (strcmp(strType, "Print To Log") == 0)
        return NODE_PRINT_TO_LOG;
    if (strcmp(strType, "Draw Debug Line") == 0)
        return NODE_DRAW_DEBUG_LINE;

    if (strcmp(strType, "Literal number") == 0)
        return NODE_LITERAL_NUMBER;
    if (strcmp(strType, "Literal string") == 0)
        return NODE_LITERAL_STRING;
    if (strcmp(strType, "Literal bool") == 0)
        return NODE_LITERAL_BOOL;
    if (strcmp(strType, "Literal color") == 0)
        return NODE_LITERAL_COLOR;

    if (strcmp(strType, "Move Camera") == 0)
        return NODE_MOVE_CAMERA;
    if (strcmp(strType, "Zoom Camera") == 0)
        return NODE_ZOOM_CAMERA;
    if (strcmp(strType, "Get Camera Center") == 0)
        return NODE_GET_CAMERA_CENTER;

    if (strcmp(strType, "Play Sound") == 0)
        return NODE_PLAY_SOUND;

    return NODE_UNKNOWN;
}

static inline char *GetKeyboardKeyName(KeyboardKey key)
{
    if (key == -1)
    {
        return "NONE";
    }

    if (key >= 'a' && key <= 'z')
    {
        key -= 32;
    }

    switch (key)
    {
    case KEY_APOSTROPHE:
        return "'";
    case KEY_COMMA:
        return ",";
    case KEY_MINUS:
        return "-";
    case KEY_PERIOD:
        return ".";
    case KEY_SLASH:
        return "/";
    case KEY_ZERO:
        return "0";
    case KEY_ONE:
        return "1";
    case KEY_TWO:
        return "2";
    case KEY_THREE:
        return "3";
    case KEY_FOUR:
        return "4";
    case KEY_FIVE:
        return "5";
    case KEY_SIX:
        return "6";
    case KEY_SEVEN:
        return "7";
    case KEY_EIGHT:
        return "8";
    case KEY_NINE:
        return "9";
    case KEY_SEMICOLON:
        return ";";
    case KEY_EQUAL:
        return "=";
    case KEY_A:
        return "A";
    case KEY_B:
        return "B";
    case KEY_C:
        return "C";
    case KEY_D:
        return "D";
    case KEY_E:
        return "E";
    case KEY_F:
        return "F";
    case KEY_G:
        return "G";
    case KEY_H:
        return "H";
    case KEY_I:
        return "I";
    case KEY_J:
        return "J";
    case KEY_K:
        return "K";
    case KEY_L:
        return "L";
    case KEY_M:
        return "M";
    case KEY_N:
        return "N";
    case KEY_O:
        return "O";
    case KEY_P:
        return "P";
    case KEY_Q:
        return "Q";
    case KEY_R:
        return "R";
    case KEY_S:
        return "S";
    case KEY_T:
        return "T";
    case KEY_U:
        return "U";
    case KEY_V:
        return "V";
    case KEY_W:
        return "W";
    case KEY_X:
        return "X";
    case KEY_Y:
        return "Y";
    case KEY_Z:
        return "Z";
    case KEY_LEFT_BRACKET:
        return "[";
    case KEY_BACKSLASH:
        return "\\";
    case KEY_RIGHT_BRACKET:
        return "]";
    case KEY_GRAVE:
        return "`";
    case KEY_SPACE:
        return "Space";
    case KEY_ESCAPE:
        return "Escape";
    case KEY_ENTER:
        return "Enter";
    case KEY_TAB:
        return "Tab";
    case KEY_BACKSPACE:
        return "Backspace";
    case KEY_INSERT:
        return "Insert";
    case KEY_DELETE:
        return "Delete";
    case KEY_RIGHT:
        return "Right";
    case KEY_LEFT:
        return "Left";
    case KEY_DOWN:
        return "Down";
    case KEY_UP:
        return "Up";
    case KEY_PAGE_UP:
        return "PageUp";
    case KEY_PAGE_DOWN:
        return "PageDown";
    case KEY_HOME:
        return "Home";
    case KEY_END:
        return "End";
    case KEY_CAPS_LOCK:
        return "CapsLock";
    case KEY_SCROLL_LOCK:
        return "ScrollLock";
    case KEY_NUM_LOCK:
        return "NumLock";
    case KEY_PRINT_SCREEN:
        return "PrintScreen";
    case KEY_PAUSE:
        return "Pause";
    case KEY_F1:
        return "F1";
    case KEY_F2:
        return "F2";
    case KEY_F3:
        return "F3";
    case KEY_F4:
        return "F4";
    case KEY_F5:
        return "F5";
    case KEY_F6:
        return "F6";
    case KEY_F7:
        return "F7";
    case KEY_F8:
        return "F8";
    case KEY_F9:
        return "F9";
    case KEY_F10:
        return "F10";
    case KEY_F11:
        return "F11";
    case KEY_F12:
        return "F12";
    case KEY_LEFT_SHIFT:
        return "Shift";
    case KEY_LEFT_CONTROL:
        return "Ctrl";
    case KEY_LEFT_ALT:
        return "Alt";
    case KEY_LEFT_SUPER:
        return "Super";
    case KEY_RIGHT_SHIFT:
        return "RightShift";
    case KEY_RIGHT_CONTROL:
        return "RightCtrl";
    case KEY_RIGHT_ALT:
        return "RightAlt";
    case KEY_RIGHT_SUPER:
        return "RightSuper";
    case KEY_KB_MENU:
        return "Menu";
    case KEY_KP_0:
        return "Keypad 0";
    case KEY_KP_1:
        return "Keypad 1";
    case KEY_KP_2:
        return "Keypad 2";
    case KEY_KP_3:
        return "Keypad 3";
    case KEY_KP_4:
        return "Keypad 4";
    case KEY_KP_5:
        return "Keypad 5";
    case KEY_KP_6:
        return "Keypad 6";
    case KEY_KP_7:
        return "Keypad 7";
    case KEY_KP_8:
        return "Keypad 8";
    case KEY_KP_9:
        return "Keypad 9";
    case KEY_KP_DECIMAL:
        return "Keypad .";
    case KEY_KP_DIVIDE:
        return "Keypad /";
    case KEY_KP_MULTIPLY:
        return "Keypad *";
    case KEY_KP_SUBTRACT:
        return "Keypad -";
    case KEY_KP_ADD:
        return "Keypad +";
    case KEY_KP_ENTER:
        return "Keypad Enter";
    case KEY_KP_EQUAL:
        return "Keypad =";
    default:
        return "Unknown";
    }
}