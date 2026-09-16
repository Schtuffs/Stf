#pragma once

#include "Types/ShortTypes.h"
#include "Types/Types.h"

typedef struct ArrayList {
    void** data;
    u64   count;
    u64   size;
} ArrayList;

ArrayList ArrayListInit(u64 size);
bool      ArrayListIsValid(ArrayList list);
void      ArrayListDestroy(ArrayList* list);

void ArrayListClear(ArrayList* list);
bool ArrayListAdd(ArrayList* list, void* item);
