#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "colorutils.h"

#define STARTSIZE 16

int findvaluesfile(FILE *fin, Color **arr, size_t *currpos, size_t *arrlen);
int findnextvalue(FILE *f, Color *col);
int checkdup(Color *arr, size_t currpos, Color c);

int main(int argc, char **argv)
{
    FILE *infile;
    Color *colorarr;
    size_t currpos, arrlen;
    int retval = 0;
    
    currpos = 0;        /* initialize array */
    arrlen = STARTSIZE;
    colorarr = calloc(arrlen, sizeof(Color));
    if (!colorarr) {
        fputs("ERROR: Out of memory\n", stderr);
        return 1;
    }

    if (argc == 1) {        /* no arguments: get values from stdin */
        retval = findvaluesfile(stdin, &colorarr, &currpos, &arrlen);
        if (retval == 1)
            goto cleanup;
    } else {
        while (--argc) {    /* get values from every file passed as arguments */
            infile = fopen(*++argv, "r");
            if (!infile) {
                fprintf(stderr, "ERROR: %s: no such file or directory\n", *argv);
                continue;
            }
            retval = findvaluesfile(infile, &colorarr, &currpos, &arrlen);
            if (retval == 1)
                break;
            fclose(infile);
            infile = NULL;
        }
    }

    for (size_t i = 0; i < currpos; i++)
        color_print(&colorarr[i]);
cleanup:
    free(colorarr);
    if (infile)
        fclose(infile);
    return retval;
}

int findvaluesfile(FILE *fin, Color **arr, size_t *currpos, size_t *arrlen)
{
    int err;
    Color col;

    while ((err = findnextvalue(fin, &col)) != EOF) {
        if (checkdup(*arr, *currpos, col))
            continue;
        if (*currpos == *arrlen) {
            (*arrlen) *= 2;
            *arr = reallocarray(*arr, *arrlen, sizeof(Color *));
            if (!(*arr))
                return 1;   /* memory error */
        }
        (*arr)[(*currpos)++] = col;
    }

    return 0;
}

/* Returns EOF if f is NULL or if reached end of file */
int findnextvalue(FILE *f, Color *col)
{
    char colstr[9]; /* +1 for terminating character */
    int i, c;

    if (!f)
        return EOF;

    while ((c = getc(f)) != EOF) {
        if (c != '#')
            continue;
        i = 0;
        while (isxdigit(c = getc(f)) && i != 8) /* collect value */
            colstr[i++] = toupper(c);
        colstr[i] = '\0';
        if (!color_iscolor(colstr))
            continue;
        color_strtocolor(colstr, col);
        return 0;
    }

    return EOF;
}

int checkdup(Color *arr, size_t currpos, Color c)
{
    size_t i;
    if (!arr || currpos == 0)
        return 0;
    for (i = 0; i < currpos; i++) {
        if (color_colorcmp(&c, &arr[i]))
            return 1;   /* found */
    }
    return 0;   /* not found */
}

