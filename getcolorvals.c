#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "colorutils.h"

/* function prototypes */
int findvalues(FILE *f);

int main(int argc, char **argv)
{
    FILE *f;
    int retval;
    
    if (colorlist_init() == 2)
        goto memerr;

    if (argc == 1) {        /* no arguments: get values from stdin */
        retval = findvalues(stdin);
        if (retval)
            goto memerr;
    } else {
        while (--argc) {    /* get values from every file passed as arguments */
            f = fopen(*++argv, "r");
            if (!f) {
                fprintf(stderr, "ERROR: Couldn't open file %s\n", *argv);
                continue;
            }
            retval = findvalues(f);
            if (retval)
                goto memerr;
            fclose(f);
        }
    }
    
    colorlist_print();
    colorlist_free();
    return 0;

memerr:
    fputs("ERROR: Out of memory\n", stderr);
    colorlist_free();
    return 1;
}

/* returns 1 for memory error */
int findvalues(FILE *f)
{
    char color[9]; /* +1 for terminating character */
    int i, c;

    while ((c = getc(f)) != EOF) { /* get values */
        if (c == '#') {
            i = 0;
            while (ishexdigit(c = getc(f)) && i != 8) /* collect value */
                color[i++] = toupper(c);
            color[i] = '\0';

            if (!iscolor(color))
                continue;
            
            if (colorlist_insert_str(color) == 2)
                return 1;
        }
    }
    return 0;
}

