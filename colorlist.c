/* ******************************************************
 *            colorlist.c
 * A very small library to organize lists of color values
 * used by getcolorvals.c and makepal.c
 *
 * ******************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long    list_lenght = 100;  /* self explanatory */
static char           **color_list;         /* used for storing strings of color values */
static size_t           currpos = 0;        /* points to first free pos */

/* colorlistinit: initializes the color list. call this before doing operations. */
int colorlist_init(void)
{
    color_list = calloc(list_lenght, sizeof(char *));
    if (!color_list)
        return 1;
    return 0;
}

/* checkdup: check if n already appears in cval_list array
 * returns 1 if it appears, 0 if not */
int colorlist_checkdup(char *v)
{
    for (int i = 0; i < currpos; i++) {
        if (strcmp(color_list[i], v) == 0)
            return 1; /* found */
    }
    return 0; /* not found */
}

/* insertcolorval: insert a value into cval_list array
 * returns 1 if out of memory */
int colorlist_insert(char *v)
{
    if (currpos == list_lenght) {
        list_lenght *= 2;
        color_list = realloc(color_list, list_lenght);
        if (!color_list)
            return 1;
    }
    color_list[currpos++] = strdup(v);
    return 0;
}

/* printcolorlist: print all the colors in the list */
void colorlist_print(void)
{
    for (int i = 0; i < currpos; i++)
        printf("%s\n", color_list[i]);
}

/* freecolorlist: free the color_list array. */
void colorlist_free(void)
{
    for (int i = 0; i < currpos; i++)
        free(color_list[i]);
    free(color_list);
}
