/* *****************************************************************
 *                      makepal.c
 * A very simple program which takes a list of colors (from file or
 * standard input) and creates an image with those colors.
 * Only PNG images are supported.
 *
 * *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "pngimage.h"
#include "color.h"
#include "autoarray.h"

#define DEBUG

int readcolor(FILE *stream, Color *c);
int writeimage(FILE *fimg, Color **arr, size_t arrlen);

#ifdef _WIN32   /* Windows doesn't have a getline function. */
typedef intptr_t ssize_t;
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#define STARTLEN 10
#define IMGNAME "palette.png"

/* Gets the next color from stream.
 * 
 * Stream is a file containing a list which should be formatted like this:
 * 0CFA2E25
 * 09BC6751
 * ...
 * Blank lines are skipped.
 * 
 * Returns 0 for success, 1 for list format error, 2 for getline failure, 
 * 3 for bad argument */
int readcolor(FILE *stream, Color *c)
{
    ssize_t llen;
    char *line = NULL;
    size_t n = 0;
    int ret = 0;
    
    if (!stream || !c)
        return 3;               /* bad arg */

    llen = getline(&line, &n, stream);  /* get next line and examine it */
    if (llen == -1) {
        ret = 2;                /* getline failure */
        goto cleanup;
    }
    if (line[llen-1] == '\n')   /* remove newline */
        line[llen-1] = '\0';
    if (!color_iscolor(line)) {
        ret = 1;                /* list format error */
        goto cleanup;
    }
    color_strtocolor(line, c);  /* convert to color */

cleanup:
    if (line)
        free(line);
    return ret;
}

/* Writes the colors contained in arr into an image file.
 * Returns 1 for general error, 2 for memory
 * error, 3 for libpng problem */
int writeimage(FILE *fimg, Color **arr, size_t arrlen)
{
    mainprog_info maininfo;
    int err, ret = 0;
    size_t i, j;
    
    if (!fimg || !arr || arrlen == 0)
        return 1;
    
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
        ret = 3;    /* libpng problem */
        goto cleanup;
    } else if (err == 4) {
        ret = 2;    /* memory error */
        goto cleanup;
    }
 
    maininfo.image_data = malloc(arrlen*4*sizeof(char));
    if (!maininfo.image_data) {
        ret = 2;    /* memory error */
        goto cleanup;
    }

    /* Fill data for one row and encode the row.
     * due to the nature of this program, we just need to set up one row.
     * To get an image with more than one row, you need to loop over
     * rows too. */
    for (i = 0, j = 0; i < arrlen; i++) {   /* fill image_data */
        maininfo.image_data[j++] = arr[i]->red;
        maininfo.image_data[j++] = arr[i]->green;
        maininfo.image_data[j++] = arr[i]->blue;
        maininfo.image_data[j++] = arr[i]->alpha;
    }

    err = 0;
    err = writepng_encode_row(&maininfo);
    err = writepng_encode_finish(&maininfo);
    if (err == 2) {
        ret = 3;    /* libpng error */
        goto cleanup;
    }

cleanup:
    fclose(fimg);
    if (maininfo.image_data)
        free(maininfo.image_data);
    writepng_cleanup(&maininfo);
    return ret;
}

/* An implementation of getline for Windows.
 * It's not very fast, improvements are welcome. */
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

/* Checks for duplicates in a color array.
 * Returns 1 if found, 0 if not found or if arr is empty. */
int checkdup(Color **arr, size_t currpos, Color c)
{
    size_t i;
    if (!arr || currpos == 0)
        return 0;
    for (i = 0; i < currpos; i++) {
        if (color_colorcmp(&c, arr[i]))
            return 1;   /* found */
    }
    return 0;   /* not found */
}

int main(int argc, char **argv)
{
    FILE *infile, *fimg;
    int ret, err;
    Color **colorarr, col;
    size_t currpos, arrlen, linen;
    
    if (argc > 2) {
        printf("Usage: %s [LIST FILE]\n", *argv);
        return 1;
    }

    infile = fimg = NULL;
    ret = err = 0;

    currpos = 0;    /* allocate array of colors */
    arrlen = STARTLEN;
    colorarr = calloc(arrlen, sizeof(Color *));
    if (!colorarr) {
        fputs("ERROR: Out of memory\n", stderr);
        return 1;
    }

    if (argc == 1)  /* if no argument, read from standard input */
        infile = stdin;
    else if (argc == 2) {
        infile = fopen(argv[1], "r");
        if (!infile) {
            fprintf(stderr, "ERROR: %s: no such file or directory", 
                    argv[1]);
            ret = 1;
            goto cleanup;
        }
    }

    /* read next color from stream and insert into the color array */
    linen = 0;  /* keep the line number for error message */
    while ((err = readcolor(infile, &col)) == 0) {
        linen++;
        if (checkdup(colorarr, currpos, col))   /* check for duplicates */
            continue;
        if (currpos == arrlen) {    /* expand array if needed */
            arrlen << 1; /* multiply by 2 */
            colorarr = reallocarray(colorarr, arrlen, sizeof(Color *));
            if (!colorarr) {
                fputs("ERROR: Out of memory\n", stderr);
                ret = 1;
                goto cleanup;
            }
        }
        colorarr[currpos++] = color_colordup(&col); /* insert */
    }
    
    if (err == 1) {
        fprintf(stderr, "ERROR: list format error at line %ld\n", linen);
        ret = 1;
        goto cleanup;
    }

#ifdef DEBUG
    for (size_t i = 0; i < currpos; i++)
        color_print(colorarr[i]);
#endif

    fimg = fopen(IMGNAME, "wb");    /* open image file */
    if (!fimg) {
        fprintf(stderr, "ERROR: Can't open %s for writing\n", IMGNAME);
        ret = 1;
        goto cleanup;
    }
    
    /* write color array to image */
    err = writeimage(fimg, colorarr, currpos);
    switch (err) {
    case 2:
        fputs("ERROR: Out of memory\n", stderr);
        ret = 1;
        goto cleanup;
    case 3:
        fputs("ERROR: lipng error\n", stderr);
        ret = 1;
        goto cleanup;
    default:
        printf("Wrote list to %s file\n", IMGNAME);
    }
 
cleanup:
    if (colorarr) {
        for (size_t i = 0; i < currpos; i++)
            free(colorarr[i]);
        free(colorarr);
    }
    if (infile)
        fclose(infile);
    if (fimg)
        fclose(fimg);
    return ret;
}

