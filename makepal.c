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
#define error(...) do { fprintf(stderr, "error: " __VA_ARGS__); } while (0)
#define STARTLEN 10
#define IMGNAME "palette.png"

int readcolor(FILE *stream, Color *c);
int writeimage(const char *fname, AutoArray *arr);
int process(FILE *infile, const char *name);
Color *dupcolor(Color c);

#ifdef _WIN32   /* Windows doesn't have a getline function. */
typedef intptr_t ssize_t;
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

/* Gets the next color from stream.
 * Stream is a file containing a list which should be formatted like this:
 * 0CFA2E25
 * 09BC6751
 * ...
 * Blank lines are skipped.
 * Returns 0 for success, 1 for list format error, 2 for getline failure */
int readcolor(FILE *stream, Color *cptr)
{
    ssize_t llen;
    char *line = NULL;
    size_t n = 0;
    int ret = 0;

    llen = getline(&line, &n, stream);  /* get next line and examine it */
    if (llen == -1) {
        ret = 2;                /* getline failure */
        goto cleanup;
    }
    if (line[llen-1] == '\n')   /* remove newline */
        line[llen-1] = '\0';
    if (color_strtocolor(line, cptr) != 0) {
        ret = 1;                /* list format error */
        goto cleanup;
    }

cleanup:
    if (line)
        free(line);
    return ret;
}

/* Writes the colors contained in arr into an image file.
 * Returns 1 for general error, 2 for memory error, 3 for libpng problem */
int writeimage(const char *fname, AutoArray *arr)
{
    int err;
    size_t i, j;
    Image img = { .w = AUTOARR_SIZE(arr), .h = 1, .ch = 4};
    FILE *outfile;

    /* set up output file */
    outfile = fopen(fname, "w");
    if (!outfile)
        return 1;

    /* set up image data */
    img.data = malloc(AUTOARR_SIZE(arr)*4*sizeof(char));
    if (!img.data) {
        err = 2;
        goto cleanup;
    }
    for (i = 0, j = 0; i < AUTOARR_SIZE(arr); i++) {
        Color *tmp = (Color *) AUTOARR_GET(arr, i);
        img.data[j++] = tmp->red;
        img.data[j++] = tmp->green;
        img.data[j++] = tmp->blue;
        img.data[j++] = tmp->alpha;
    }

    err = pngimage_write_image_rgba(&img, outfile);

    // err = writepng_init(&maininfo, PNG_COLOR_TYPE_RGBA);
    // if (err == 2) {
    //     ret = 3;    /* libpng problem */
    //     goto cleanup;
    // } else if (err == 4) {
    //     ret = 2;    /* memory error */
    //     goto cleanup;
    // }

    err = 0;
cleanup:
    fclose(outfile);
    if (img.data)
        free(img.data);
    pngimage_write_cleanup(&img);
    return err;
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

/* returns non-zero for any important error */
int process(FILE *infile, const char *name)
{
    size_t linen;
    int err;
    Color c;
    AutoArray *autarr;

    linen = err = 0;
    autarr = autoarr_make();
    if (!autarr)
        return 1;

    while (err = readcolor(infile, &c), err == 0) {
        if (autoarr_find(autarr, &c, color_compare) != NULL)
            continue;
        if (autoarr_append(autarr, dupcolor(c)) == AUTOARR_ERR_NOMEM) {
            err = 2;
            goto cleanup;
        }
    }
    if (err == 1) {
        error("line %ld: list format error\n", linen);
        err = 0;
        goto cleanup;
    }

    err = writeimage(name, autarr);
    switch (err) {
    case 1:
        error("can't open %s for writing\n", IMGNAME);
        err = 0;
        goto cleanup;
    case 2: err = 2; goto cleanup;
    case 3: err = 3; goto cleanup;
    default:
#ifdef DEBUG
        fprintf(stderr, "wrote list to %s file\n", IMGNAME);
#endif
    }
    
    err = 0;
cleanup:
    for (int i = 0; i < AUTOARR_SIZE(autarr); i++) {
        Color *tmp = (Color *) AUTOARR_GET(autarr, i);
        free(tmp);
    }
    autoarr_free(autarr);
    return err;
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
    FILE *infile;
    int err;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [LIST FILE]\n", *argv);
        return 1;
    }

    infile = NULL;
    if (argc == 1) {
        err = process(stdin, IMGNAME);
    } else {
        while (++argv, --argc > 0) {
            infile = fopen(*argv, "r");
            if (!infile) {
                error("%s: no such file or directory", argv[1]);
                return 1;
            }
            err = process(infile, IMGNAME);
            fclose(infile);
            if (err != 0)
                break;
        }
    }

    if (err != 0) {
        switch (err) {
        case 2: error("out of memory\n"); break;
        case 3: error("libpng error\n"); break;
        }
        return 1;
    }
    return 0;
}

