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
#define IMGNAME "palette.png"

enum {
    ERR_BADPARAM = 1,
    ERR_FILE,
    ERR_NOMEM,
    ERR_LIBPNG,
};

int readcolor(FILE *stream, Color *c);
int writeimage(const char *fname, AutoArray *arr);
int process(FILE *infile, const char *name);
void free_color_arr(AutoArray *autarr);
void die(int err);

#ifdef _WIN32   /* Windows doesn't have a getline function. */
typedef intptr_t ssize_t;

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

/* Gets the next color from stream.
 * Stream is a file containing a list which should be formatted like this:
 * 0CFA2E25
 * 09BC6751
 * ...
 * Blank lines are skipped. */
int readcolor(FILE *stream, Color *cptr)
{
    ssize_t llen;
    char *line = NULL;
    size_t n = 0;

    llen = getline(&line, &n, stream);  /* get next line and examine it */
    if (llen == -1)
        return 2;
    if (line[llen-1] == '\n')   /* remove newline */
        line[llen-1] = '\0';
    if (color_strtocolor(line, cptr) != 0) {
        free(line);
        return 1;
    }
    free(line);
    return 0;
}

/* returns ERR_FILE, ERR_NOMEM, ERR_LIBPNG */
int writeimage(const char *fname, AutoArray *arr)
{
    int err;
    size_t i, j;
    Image img = { .w = AUTOARR_SIZE(arr), .h = 1, .ch = 4};
    FILE *outfile;

    /* set up output file */
    outfile = fopen(fname, "w");
    if (!outfile)
        return ERR_FILE;

    /* set up image data */
    img.data = malloc(AUTOARR_SIZE(arr)*4*sizeof(char));
    if (!img.data) {
        fclose(outfile);
        return ERR_NOMEM;
    }

    for (i = 0, j = 0; i < AUTOARR_SIZE(arr); i++) {
        Color *tmp = (Color *) AUTOARR_GET(arr, i);
        img.data[j++] = tmp->red;
        img.data[j++] = tmp->green;
        img.data[j++] = tmp->blue;
        img.data[j++] = tmp->alpha;
    }

    /* write image */
    err = pngimage_write_image_rgba(&img, outfile);
    if (err != 0) {
        fclose(outfile);
        free(img.data);
        switch (err) {
        case IMAGE_ERR_NOMEM: return ERR_NOMEM;
        case IMAGE_ERR_GENERIC: return ERR_LIBPNG;
        }
    }

    fclose(outfile);
    free(img.data);
    return 0;
}

/* returns non-zero for any important error */
int process(FILE *infile, const char *name)
{
    size_t linen = 0;
    int err = 0;
    Color c;
    AutoArray *autarr;

    /* init auto array */
    autarr = autoarr_make();
    if (!autarr)
        return 1;

    /* read file and get color */
    while (err = readcolor(infile, &c), err == 0) {
        if (autoarr_find(autarr, &c, color_compare) != NULL)
            continue;
        if (autoarr_append(autarr, color_dup(c)) == AUTOARR_ERR_NOMEM) {
            free_color_arr(autarr);
            return ERR_NOMEM;
        }
    }
    if (err == 1) {
        error("%ld: format error\n", linen);
        free_color_arr(autarr);
        return 0;
    }

    /* write resulting image */
    err = writeimage(name, autarr);
    if (err != 0) {
        free_color_arr(autarr);
        return err;
    }

    fprintf(stderr, "wrote list to %s file\n", IMGNAME);
    free_color_arr(autarr);
    return 0;
}

void free_color_arr(AutoArray *autarr)
{
    Color *tmp;
    for (size_t i = 0; i < AUTOARR_SIZE(autarr); i++) {
        tmp = (Color *) AUTOARR_GET(autarr, i);
        free(tmp);
    }
    autoarr_free(autarr);
}

void die(int err)
{
    switch (err) {
    case 1: error("can't open %s for writing\n", IMGNAME); break;
    case 2: error("out of memory\n"); break;
    case 3: error("libpng error\n"); break;
    }
    // remember: exit flushes and closes all open files
    exit(1);
}

int main(int argc, char **argv)
{
    FILE *infile = NULL;
    int err = 0;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [LIST FILE]\n", *argv);
        return 1;
    }

    if (argc == 1) {
        err = process(stdin, IMGNAME);
        if (err != 0)
            die(err);
    } else {
        while (++argv, --argc > 0) {
            infile = fopen(*argv, "r");
            if (!infile) {
                error("%s: no such file or directory", argv[1]);
                return 1;
            }
            err = process(infile, IMGNAME);
            if (err != 0)
                die(err);
            fclose(infile);
        }
    }
    return 0;
}

