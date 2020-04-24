/* **********************************************************
 *                      makepal.c
 * A very simple program which takes a list of colors (from 
 * file or standard input) and creates an image with those
 * colors.
 * Only PNG images are supported.
 *
 * ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <png.h>
#include <zlib.h>

#include "writepng.h"
#include "colorutils.h"

//#define DEBUG

int readcolors(FILE *infile, Color **arr, size_t *currpos, size_t *arrlen);
int writeimage(Color **arr, size_t arrlen);
int checkdup(Color **arr, size_t currpos, Color c);

#ifdef _WIN32   /* Windows doesn't have a getline function. */
typedef intptr_t ssize_t;
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#define STARTLEN 16
#define IMGNAME "palette.png"

int main(int argc, char **argv)
{
    FILE *infile;
    Color **colorarr;
    size_t currpos, arrlen;
    int err;
    
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [LIST FILE]\n", *argv);
        return 1; 
    }

    currpos = 0;    /* allocate array of colors */
    arrlen = STARTLEN;
    colorarr = calloc(arrlen, sizeof(Color));
    if (!colorarr) {
        fputs("ERROR: Out of memory\n", stderr);
        return 1;
    }

    if (argc == 1) {  /* read from stdin */
        err = readcolors(stdin, colorarr, &currpos, &arrlen);
        if (err == 1 || err == 2) {
            free(colorarr);
            return 1;
        }
    } else if (argc == 2) { /* read from file */
        infile = fopen(argv[1], "r");
        if (!infile) {
            fprintf(stderr, "ERROR: couldn't open file %s\n", argv[1]);
            return 1;
        }

        err = readcolors(infile, colorarr, &currpos, &arrlen);
        if (err == 1 || err == 2) {
            free(colorarr);
            return 1;
        }
    }
    
#ifdef DEBUG
    for (size_t i = 0; i < currpos; i++)
        color_print(colorarr[i]);
#endif

    writeimage(colorarr, currpos);
    free(colorarr);

    //fputs("ERROR: out of memory\n", stderr);
    //fputs("ERROR: list format error\n", stderr);
    //fputs("ERROR: libpng error\n", stderr);
    
    return 0;
}

/* Gets colors from a file with a list of colors and puts them into
 * an array of colors.
 *
 * The list should be formatted like this:
 * 0CFA2E25
 * 09BC6751
 * ...
 * Blank lines are skipped.
 *
 * If arr is NULL and currpos or arrlen is 0, then a new array will
 * be allocated. The new array should later be freed.
 *
 * Returns 0 on success, 1 on list format error, 2 on memory error,
 * 3 if any argument is NULL. */
int readcolors(FILE *infile, Color **arr, size_t *currpos, size_t *arrlen)
{
    char *line;
    size_t n, llen;
    Color c;
    
    if (!infile || !arr || !currpos || !arrlen) {
        fputs("ERROR: readcolors: NULL arg\n", stderr);
        return 3;
    }

    line = NULL;
    while((llen = getline(&line, &n, infile)) != -1) {
        if (line[llen-1] == '\n') /* remove newline */
            line[llen-1] = '\0';

        if (!color_iscolor(line)) {
            fputs("ERROR: readcolors: list format error\n", stderr);
            free(line);
            return 1;
        }

        color_strtocolor(line, &c);     /* convert to color */
        if (checkdup(arr, *currpos, c))
            continue;
        if (*currpos == *arrlen) {      /* reallocate array if needed */
            (*arrlen) *= 2;
            arr = reallocarray(arr, *arrlen, sizeof(Color));
            if (!arr) {
                fputs("ERROR: readcolors: out of memory\n", stderr);
                free(line);
                return 2;
            }
        }
        arr[(*currpos)++] = color_colordup(&c); /* insert */
    }
    free(line);

    return 0;
}

/* Writes the colors contained in arr into an image file.
 * Returns 1 for general errors, 2 for memory
 * error, 3 for other errors. */
int writeimage(Color **arr, size_t arrlen)
{
    FILE *fimg;
    mainprog_info maininfo;
    int err;
    size_t i, j;
    
    if (!arr || arrlen == 0) {
        fputs("ERROR: writeimage: error with arguments\n", stderr);
        return 1;
    }

    fimg = fopen(IMGNAME, "wb");
    if (!fimg) {
        fputs("ERROR: writeimage: can't open "IMGNAME"\n", stderr);
        return 1;
    }

    /* Set up the simplest possible mainprog_info: 
     * we want only an IHDR chunk, an IDAT chunk and an IEND chunk */
    maininfo.outfile      = fimg;
    maininfo.width        = arrlen;
    maininfo.height       = 1;
    maininfo.sample_depth = 8;
    maininfo.filter       = 0;
    maininfo.interlaced   = 0;
    maininfo.have_bg      = 0;
    maininfo.have_time    = 0;
    maininfo.have_text    = 0;
    maininfo.gamma        = 0.0;

    err = writepng_init(&maininfo, PNG_COLOR_TYPE_RGBA);
    if (err == 2) {
        fputs("ERROR: writeimage: libpng error\n", stderr);
        return 3;
    } else if (err == 4) {
        fputs("ERROR: writeimage: out of memory\n", stderr);
        return 2;
    }
    
    maininfo.image_data = malloc(arrlen*4*sizeof(char));
    if (!maininfo.image_data) {
        fputs("ERROR: writeimage: memory error\n", stderr);
        return 2;
    }

    for (i = 0, j = 0; i < arrlen; i++) {   /* fill image_data */
        maininfo.image_data[j++] = arr[i]->red;
        maininfo.image_data[j++] = arr[i]->green;
        maininfo.image_data[j++] = arr[i]->blue;
        maininfo.image_data[j++] = arr[i]->alpha;
    }

    /* due to the nature of this program, we just need to set up one row.
     * to get an image with more than one row, you'd have to do another 
     * loop, one which loops over each row */

    /* get image data for one row and call writepng_encode_row() */
    if (writepng_encode_row(&maininfo) == 2) {
        fputs("ERROR: writeimage: memory error\n", stderr);
        return 2;
    }

    if (writepng_encode_finish(&maininfo) == 2) {
        fputs("ERROR: writeimage: memory error\n", stderr);
        return 2;
    }

    printf("Wrote list to %s file\n", IMGNAME);

cleanup:
    fclose(fimg);
    if (maininfo.image_data)
        free(maininfo.image_data);
    writepng_cleanup(&maininfo);
    return 0;
}

#ifdef _WIN32
ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    size_t pos;
    int c;
    
    if (lineptr == NULL || stream == NULL || n == NULL)
        return -1;
    
    if ((c = getc(stream)) == EOF)
        return -1;
    
    if (*lineptr == NULL) {
        *lineptr = malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL)
                return -1;
            *n = new_size;
            *lineptr = new_ptr;
        }

        (*lineptr)[pos++] = c;
        if (c == '\n')
            break;
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}
#endif

int checkdup(Color **arr, size_t currpos, Color c)
{
    for (int i = 0; i < currpos; i++) {
        if (color_colorcmp(&c, arr[i]))
            return 1;   /* found */
    }
    return 0;   /* not found */
}
