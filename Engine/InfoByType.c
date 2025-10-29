#include "InfoByType.h"

const char *menuItems[] = {"Variable", "Event", "Get", "Set", "Flow", "Sprite", "Draw Prop", "Logical", "Debug", "Literal", "Camera", "Sound"};
const char *subMenuItems[][subMenuItemCount] = {
    {"Create number", "Create string", "Create bool", "Create color", "Cast to number", "Cast to string", "Cast to bool", "Cast to color"},
    {"Event Start", "Event Tick", "Event On Button"},
    {"Get variable", "Get Screen Width", "Get Screen Height", "Get Mouse Position", "Get Random Number", "Get Sprite Position"},
    {"Set variable", "Set Background", "Set FPS"},
    {"Branch", "Loop", "Flip Flop", "Break", "Sequence"},
    {"Create sprite", "Spawn sprite", "Destroy sprite", "Set Sprite Position", "Set Sprite Rotation", "Set Sprite Texture", "Set Sprite Size", "Force"},
    {"Draw Prop Rectangle", "Draw Prop Circle"},
    {"Comparison", "Gate", "Arithmetic", "Clamp", "Lerp", "Sin", "Cos"},
    {"Print To Log", "Draw Debug Line", "Comment"},
    {"Literal number", "Literal string", "Literal bool", "Literal color"},
    {"Move Camera", "Zoom Camera", "Get Camera Center", "Shake Camera"},
    {"Play Sound"}};

const int subMenuCounts[] = {8, 3, 6, 3, 5, 8, 2, 7, 3, 4, 4, 1};