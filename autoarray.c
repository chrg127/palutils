#include "autoarray.h"

#include <stdio.h>
#include <stdlib.h>

#define INIT_MAX_SIZ BUFSIZ

AutoArray *autoarr_make(void)
{
    void **arr;
    AutoArray *autoarr;

    arr = calloc(INIT_MAX_SIZ, sizeof(void *));
    if (!arr)
        return NULL;
    autoarr = malloc(sizeof(AutoArray));
    if (!autoarr)
        return NULL;
    autoarr->arr = arr;
    autoarr->s = 0;
    autoarr->max = INIT_MAX_SIZ;
    return autoarr;
}

int autoarr_append(AutoArray *arr, void *elem)
{
    if (!arr)
        return AUTOARR_ERR_BADPARAM;
    if (arr->s == arr->max) {
        arr->max *= 2;
        arr->arr = realloc(arr->arr, arr->max * sizeof(void *));
        if (!arr->arr)
            return AUTOARR_ERR_NOMEM;
    }
    arr->arr[arr->s++] = elem;
    return 0;
}

void *autoarr_change(AutoArray *arr, size_t i, void *data)
{
    void *elem;

    if (!arr)
        return NULL;
    elem = arr->arr[i];
    arr->arr[i] = data;
    return elem;
}

void *autoarr_removelast(AutoArray *arr)
{
    if (!arr)
        return NULL;
    void *elem = arr->arr[arr->s];
    arr->s--;
    return elem;
}

void *autoarr_find(AutoArray *arr, const void *data,
            int (*compar)(const void *, const void *))
{
    size_t i;

    if (!arr || !data || !compar || arr->s == 0)
        return NULL;
    for (i = 0; i < arr->s; i++) {
        if ((*compar)(arr->arr[i], data) == 0)
            return arr->arr[i]; /* found */
    }
    return NULL;
}

void autoarr_free(AutoArray *arr)
{
    free(arr->arr);
    free(arr);
}

