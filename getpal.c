/* **********************************************************
 *                      getpal.c
 * A very simple program which takes one or more images and
 * prints its palette to stdout.
 * The palette is a list of colors where every element is 
 * unique.
 * Only PNG images are supported.
 * 
 * ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "pngimage.h"
#include "color.h"

#define error(...) do { fprintf(stderr, "error: " __VA_ARGS__); } while (0)

const Image pngimage_default = { NULL, 0, 0, NULL, NULL, 0, 0, 0 };

int printcolors(Image *img);
int checkdup(Color *arr, size_t currpos, Color c);

/* Gets and printf every color in an image. It needs the image's data.
 * Returns 1 for memory error. */
int printcolors(Image *img)
{
    unsigned char *data, *data_end;
    Color         *colorarr, col;
    size_t        currpos;

    currpos = 0;
    colorarr = calloc(img->w * img->h, sizeof(Color));
    if (!colorarr)
        return 1; /* memory error */

    /* The image data is composed of bytes representing every pixel in the image.
     * The pixels in turn are represented of red, green and blue values. If there are
     * 4 channels, there's an alpha value too and must be taken in consideration. */
    data = img->data;
    data_end = img->data + img->w * img->h * img->ch;
    col.alpha = 0xFF;
    while(data < data_end) {     /* read data and get color values */
        col.red = *data++;
        col.green = *data++;
        col.blue = *data++;
        if (img->ch == 4)
            col.alpha = *data++;
        if (checkdup(colorarr, currpos, col))
            continue;
        colorarr[currpos++].value = col.value;
    }

    for (size_t i = 0; i < currpos; i++)
        printf("%08X\n", colorarr[i].value);
    free(colorarr);
    return 0;
}

int checkdup(Color *arr, size_t currpos, Color c)
{
    if (!arr || currpos == 0)
        return 0;
    for (size_t i = 0; i < currpos; i++)
        if (arr[i].value == c.value)
            return 1;   /* found */
    return 0;   /* not found */
}

int main(int argc, char **argv)
{
    FILE *infile;
    int err;
    Image img;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [image files...]\n", *argv);
        return 1;
    }

    /* parse arguments */
    while (++argv, --argc > 0) {
        img = pngimage_default;
        infile = fopen(*argv, "rb");
        if (!infile) {
            error("couldn't open %s\n", *argv);
            continue;
        }

        err = pngimage_read_image(&img, infile);
        switch (err) {
        case IMAGE_ERR_NOTIMAGE:
            error("%s: not an image file\n", *argv);
            fclose(infile);
            continue;
        case IMAGE_ERR_NOMEM:
            error("out of memory\n");
            return 1;
        case IMAGE_ERR_GENERIC:
            error("libpng error\n");
            return 1;
        }

        err = printcolors(&img);
        if (err != 0) {
            error("out of memory\n");
            return 1;
        }
        
        free(img.data);
        fclose(infile);
    }
    return 0;
}

