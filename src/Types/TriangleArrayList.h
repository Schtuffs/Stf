#pragma once

#include "Types/ShortTypes.h"
#include "Types/Types.h"

typedef struct TriangleArrayList {
    Triangle* data;
    u64       count;
    u64       size;
} TriangleArrayList;

TriangleArrayList TriangleArrayListInit(u64 size);
bool              TriangleArrayListIsValid(TriangleArrayList list);
void              TriangleArrayListDestroy(TriangleArrayList* list);

void TriangleArrayListClear(TriangleArrayList* list);
bool TriangleArrayListAdd(TriangleArrayList* list, Triangle triangle);
