#pragma once

#include "Types/ShortTypes.h"
#include "Types/Types.h"

typedef struct ColourArrayList {
    Colour* data;
    u64     count;
    u64     size;
} ColourArrayList;

ColourArrayList ColourArrayListInit(u64 size);
bool            ColourArrayListIsValid(ColourArrayList list);
void            ColourArrayListDestroy(ColourArrayList* list);

void ColourArrayListClear(ColourArrayList* list);
bool ColourArrayListAdd(ColourArrayList* list, Colour colour);
