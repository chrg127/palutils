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

//#define DEBUG

#include "readpng.h"
#include "colorutils.h"

int getcolors(FILE *img, char *fname);

int main(int argc, char **argv)
{
    FILE *image_file;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [image files...]\n", *argv);
        return 1;
    }
    if (colorlist_init() == 2)
        goto memerr;

    while (--argc > 0) {    /* parse arguments */
        ++argv;
        image_file = fopen(*argv, "rb");
        if (!image_file) {
            fprintf(stderr, "ERROR: Can't read %s\n", argv[1]);
            continue;
        }
        if (getcolors(image_file, *argv) == 1)
            goto memerr;
        fclose(image_file);
    }

    colorlist_print();
    colorlist_free();

    return 0;

memerr:
    fputs("ERROR: Out of memory\n", stderr);
    colorlist_free();
    return 1;
}

/* getcolors: read the PNG image file and extract its colors. returns 1 for memory error */
int getcolors(FILE *img, char *fname)
{
    int           err, channels;
    unsigned long width, height, rowbytes;
    unsigned char red, green, blue, alpha, *image_data, *data_end;
    
    /* init gets the width and height of the PNG file and does a few error checks */
    err = readpng_init(img, &width, &height);
    switch(err) {
    case 1:
        fprintf(stderr, "ERROR: %s: Not a PNG file\n", fname);
        goto cleanup;
    case 2:
        fprintf(stderr, "ERROR: %s: Corrupted PNG file\n", fname);
        goto cleanup;
    case 4:  /* memory error */
        return 1;
    default: /* no error */
        break;
    }
    
    /* the image data is composed of bytes representing every pixel in the image.
     * the pixels in turn are represented of red, green and blue values.
     * if there are 4 channels, there's an alpha value, too, and must be taken in consideration. */
    image_data = readpng_get_image(&channels, &rowbytes); /* get image data */
    if (!image_data)
        return 1; /* memory error */
    data_end = image_data + width*height*channels; /* find where it ends */

    alpha = 0xFF;   /* with no alpha channel, the alpha always remains at FF */
    while(image_data < data_end) {  /* read data and get color values */
        red = *image_data++;
        green = *image_data++;
        blue = *image_data++;
        if (channels == 4)
            alpha = *image_data++;
#ifdef DEBUG
        fprintf(stderr, "getcolors: %hhx %hhx %hhx %hhx %s\n", red, green, blue, alpha);
#endif
        if (colorlist_insert_vals(red, green, blue, alpha) == 2)
            return 1; /* memory error */
    }
    readpng_cleanup(1); /* finished reading data, do cleanup */

cleanup:
    readpng_cleanup(0);

    return 0;
}

