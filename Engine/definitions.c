#include "definitions.h"

char *strmac(char *buf, size_t max_size, const char *format, ...)
{
    if (!format || max_size == 0)
    {
        STRING_ALLOCATION_FAILURE = true;
        return (buf) ? buf : strdup("");
    }

    char *temp = buf;
    bool needs_free = false;

    if (!buf)
    {
        temp = malloc(max_size);
        if (!temp)
        {
            STRING_ALLOCATION_FAILURE = true;
            return strdup("");
        }
        needs_free = true;
    }

    va_list args;
    va_start(args, format);
    int written = vsnprintf(temp, max_size, format, args);
    va_end(args);

    if (written < 0)
    {
        if (needs_free)
        {
            STRING_ALLOCATION_FAILURE = true;
            free(temp);
            return strdup("");
        }
        temp[0] = '\0';
        return temp;
    }

    if ((size_t)written >= max_size)
    {
        temp[max_size - 1] = '\0';
    }

    if (needs_free)
    {
        char *result = strdup(temp);
        free(temp);
        return result;
    }

    return temp;
}

const char *AddEllipsis(Font font, const char *text, float fontSize, float maxWidth, bool showEnd)
{
    if (MeasureTextEx(font, text, fontSize, 0).x <= maxWidth)
    {
        return text;
    }

    int len = strlen(text);
    int maxChars = 0;
    char temp[MAX_LITERAL_NODE_FIELD_SIZE];

    float ellipsisWidth = MeasureTextEx(font, "...", fontSize, 0).x;

    for (int i = 1; i <= len; i++)
    {
        if (showEnd)
        {
            strmac(temp, i, "%s", text + len - i);
        }
        else
        {
            strmac(temp, i, "%.*s", i, text);
        }

        if (MeasureTextEx(font, temp, fontSize, 0).x + ellipsisWidth > maxWidth)
        {
            break;
        }

        maxChars = i;
    }

    if (showEnd)
    {
        return TextFormat("...%s", text + len - maxChars);
    }
    else
    {
        return TextFormat("%.*s...", maxChars, text);
    }
}