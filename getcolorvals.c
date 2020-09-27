#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "color.h"
#include "autoarray.h"
#include "debug.h"

#define STARTSIZE 16

int findcolors(FILE *fin, AutoArray *arr);
int findnext(FILE *f, Color *col);
Color *dupcolor(Color c);

int findcolors(FILE *fin, AutoArray *arr)
{
    int err;
    Color col;

    while (err = findnext(fin, &col), err != EOF) {
        if (autoarr_find(arr, &col, color_compare) != NULL)
            continue;
        autoarr_append(arr, dupcolor(col));
    }
    return 0;
}

/* Returns EOF if f is NULL or if reached end of file */
int findnext(FILE *f, Color *cptr)
{
    char colstr[9]; /* +1 for terminating character */
    int i, c;

    if (!f)
        return EOF;

    while (c = getc(f), c != EOF) {
        if (c != '#')
            continue;
        i = 0;
        /* collect value */
        while (isxdigit(c = getc(f)) && i != 8)
            colstr[i++] = toupper(c);
        colstr[i] = '\0';
        if (color_strtocolor(colstr, cptr) != 0)
            continue;
        return 0;
    }

    return EOF;
}

Color *dupcolor(Color c)
{
    Color *cptr = malloc(sizeof(Color));
    if (!cptr)
        return NULL;
    cptr->value = c.value;
    return cptr;
}

int main(int argc, char **argv)
{
    FILE *infile = NULL;
    int retval = 0;
    AutoArray *autarr;

    autarr = autoarr_make();
    if (!autarr) {
        error("out of memory\n");
        return 1;
    }

    if (argc == 1) {        /* no arguments: get values from stdin */
        retval = findcolors(stdin, autarr);
        if (retval == 1)
            goto cleanup;
    } else {
        while (--argc) {    /* get values from every file passed as arguments */
            infile = fopen(*++argv, "r");
            if (!infile) {
                error("%s: no such file or directory\n", *argv);
                continue;
            }
            retval = findcolors(infile, autarr);
            if (retval == 1)
                break;
            fclose(infile);
            infile = NULL;
        }
    }

    for (size_t i = 0; i < AUTOARR_SIZE(autarr); i++) {
        Color *tmp = (Color *) AUTOARR_GET(autarr, i);
        printf("%08X\n", tmp->value);
        free(tmp);
    }

cleanup:
    autoarr_free(autarr);
    if (infile)
        fclose(infile);
    return retval;
}

