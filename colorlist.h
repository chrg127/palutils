/* header file the colorlist.c library */

#ifndef COLORLIST_H
#define COLORLIST_H

/* colorlistinit: initializes the color list. call this before doing operations.
 * return 1 if out of memory */
int colorlist_init(void);

/* checkdup: check if n already appears in cval_list array
 * returns 1 if it appears, 0 if not */
int colorlist_checkdup(char *);

/* insertcolorval: insert a value into cval_list array
 * returns 1 if out of memory */
int colorlist_insert(char *);

/* printcolorlist: print all the colors in the list */
void colorlist_print(void);

/* freecolorlist: free the color_list array. call this at the end. */
void colorlist_free(void);

#endif

