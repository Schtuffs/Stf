#include "Types/ColourArrayList.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION >= 202311L
constexpr u64    DEFAULT_ARRAY_LIST_SIZE = 10ull;
constexpr double LIST_SCALE              = 1.5;
#else
#define DEFAULT_ARRAY_LIST_SIZE 10ull
#define LIST_SCALE 1.5
#endif

ColourArrayList ColourArrayListInit(u64 size)
{
    ColourArrayList list;
    if (size < DEFAULT_ARRAY_LIST_SIZE) {
        size = DEFAULT_ARRAY_LIST_SIZE;
    }

    list.size  = size;
    list.count = 0;
    list.data  = malloc(size * sizeof(Colour));
    if (list.data == NULL) {
        ColourArrayList ret = {0};
        return ret;
    }

    return list;
}

bool ColourArrayListIsValid(ColourArrayList list) { return (list.data != NULL); }

void ColourArrayListDestroy(ColourArrayList* list)
{
    if (list == NULL) {
        return;
    }

    if (list->data != NULL) {
        free(list->data);
    }

    list->data = NULL;
}

//

static bool CheckList(ColourArrayList* list)
{
    // Has space already
    if (list->count < list->size) {
        return true;
    }

    // Add space
    u64     newSize  = list->size * LIST_SCALE;
    Colour* newSpace = realloc(list->data, newSize * sizeof(Colour));
    if (!newSpace) {
        return false;
    }

    list->data = newSpace;
    return true;
}

void ColourArrayListClear(ColourArrayList* list) { list->count = 0; }

bool ColourArrayListAdd(ColourArrayList* list, Colour colour)
{
    if (!CheckList(list)) {
        return false;
    }

    list->data[list->count++] = colour;
    return true;
}
