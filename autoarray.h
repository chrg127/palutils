/*
 * this is a simple library for auto-resizing arrays.
 * an autoarray is created using autoarray_make. the array is always completely
 * initializated and its size it set to 0.
 */

#ifndef AUTOARRAY_H_INCLUDED
#define AUTOARRAY_H_INCLUDED

#include <stddef.h>

typedef struct _autoarr {
    void **arr; // array of pointers to void
    size_t s;
    size_t max;
} AutoArray;

enum {
    AUTOARR_ERR_BADPARAM = 1,
    AUTOARR_ERR_NOMEM,
};

AutoArray  *autoarr_make();
int         autoarr_append(AutoArray *arr, void *elem);
void       *autoarr_change(AutoArray *arr, size_t i, void *data);
void       *autoarr_removelast(AutoArray *arr);
void       *autoarr_find(AutoArray *arr, const void *data,
                    int (*compar)(const void *, const void *));
void        autoarr_free(AutoArray *arr);

#define AUTOARR_GET(autoarr, i) (autoarr)->arr[(i)];
#define AUTOARR_SIZE(autoarr) (autoarr)->s

#endif
