#include "Types/TriangleArrayList.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION >= 202311L
constexpr u64    DEFAULT_ARRAY_LIST_SIZE = 10ull;
constexpr double LIST_SCALE              = 1.5;
#else
#define DEFAULT_ARRAY_LIST_SIZE 10ull
#define LIST_SCALE 1.5
#endif

TriangleArrayList TriangleArrayListInit(u64 size)
{
    TriangleArrayList list;
    if (size < DEFAULT_ARRAY_LIST_SIZE) {
        size = DEFAULT_ARRAY_LIST_SIZE;
    }

    list.size  = size;
    list.count = 0;
    list.data  = malloc(size * sizeof(Triangle));
    if (list.data == NULL) {
        TriangleArrayList ret = {0};
        return ret;
    }

    return list;
}

bool TriangleArrayListIsValid(TriangleArrayList list) { return (list.data != NULL); }

void TriangleArrayListDestroy(TriangleArrayList* list)
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

static bool CheckList(TriangleArrayList* list)
{
    // Has space already
    if (list->count < list->size) {
        return true;
    }

    // Add space
    u64       newSize  = list->size * LIST_SCALE;
    Triangle* newSpace = realloc(list->data, newSize * sizeof(Triangle));
    if (!newSpace) {
        return false;
    }

    list->data = newSpace;
    return true;
}

void TriangleArrayListClear(TriangleArrayList* list) { list->count = 0; }

bool TriangleArrayListAdd(TriangleArrayList* list, Triangle triangle)
{
    if (!CheckList(list)) {
        return false;
    }

    list->data[list->count++] = triangle;
    return true;
}
