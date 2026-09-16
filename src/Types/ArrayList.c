#include "Types/ArrayList.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION >= 202311L
constexpr u64    DEFAULT_ARRAY_LIST_SIZE = 10ull;
constexpr double LIST_SCALE              = 1.5;
#else
#define DEFAULT_ARRAY_LIST_SIZE 10ull
#define LIST_SCALE 1.5
#endif

ArrayList ArrayListInit(u64 size)
{
    ArrayList list;
    if (size < DEFAULT_ARRAY_LIST_SIZE) {
        size = DEFAULT_ARRAY_LIST_SIZE;
    }

    list.size  = size;
    list.count = 0;
    list.data  = malloc(size * sizeof(*(list.data)));
    if (list.data == NULL) {
        ArrayList ret = {0};
        return ret;
    }

    return list;
}

bool ArrayListIsValid(ArrayList list) { return (list.data != NULL); }

void ArrayListDestroy(ArrayList* list)
{
    if (list == NULL) {
        return;
    }

    ArrayListClear(list);

    if (list->data != NULL) {
        free(list->data);
    }

    list->data = NULL;
}

//

static bool CheckList(ArrayList* list)
{
    // Has space already
    if (list->count <= list->size) {
        return true;
    }

    // Add space
    u64    newSize  = list->size * LIST_SCALE;
    void** newSpace = realloc(list->data, newSize * sizeof(*(list->data)));
    if (!newSpace) {
        return false;
    }

    list->data = newSpace;
    return true;
}

void ArrayListClear(ArrayList* list)
{
    for (u64 i = 0; i < list->count; i++) {
        if (list->data[i]) {
            free(list->data[i]);
        }
    }

    list->count = 0;
}

bool ArrayListAdd(ArrayList* list, void* item)
{
    if (!CheckList(list)) {
        return false;
    }

    list->data[list->count++] = item;
    return true;
}
