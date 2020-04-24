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

#include "readpng.h"
#include "colorutils.h"

int getcolors(FILE *fimg, char *fname);
int printcolors(unsigned char *imgdata, int w, int h, int ch);
int checkdup(Color **arr, size_t currpos, Color c);

int main(int argc, char **argv)
{
    FILE *image_file;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [image files...]\n", *argv);
        return 1;
    }

    while (--argc > 0) {    /* parse arguments */
        ++argv;
        image_file = fopen(*argv, "rb");
        if (!image_file) {
            fprintf(stderr, "ERROR: Couldn't open %s\n", argv[1]);
            continue;
        }

        if (getcolors(image_file, *argv) == 1) {
            fputs("ERROR: Out of memory\n", stderr);
            return 1;
        }

        fclose(image_file);
    }
    
    return 0;
}

/* Reads the PNG image file and extract its colors. 
 * Returns 1 for memory error */
int getcolors(FILE *fimg, char *fname)
{
    int           err, channels;
    unsigned long width, height, rowbytes;
    unsigned char *imgdata;
    
    /* init gets the width and height of the PNG file and does 
     * a few error checks */
    err = readpng_init(fimg, &width, &height);
    switch(err) {
    case 1:
        fprintf(stderr, "ERROR: %s: Not a PNG file\n", fname);
        goto cleanup;
    case 2:
        fprintf(stderr, "ERROR: %s: Corrupted PNG file\n", fname);
        goto cleanup;
    case 4:  /* Memory error */
        return 1;
    }
    
    /* Get data and allocate the color array. */
    imgdata = readpng_get_image(&channels, &rowbytes);
    if (!imgdata)
        return 1;
    
    err = printcolors(imgdata, width, height, channels);
    if (err)
        return 1;

    readpng_cleanup(1); /* Finished reading data, do cleanup */

cleanup:
    readpng_cleanup(0);

    return 0;
}

/* Gets and printf every color in an image. It needs the image's data.
 * Returns 1 for memory error. */
int printcolors(unsigned char *imgdata, int w, int h, int ch)
{
    unsigned char *data_end;
    Color         **colorarr, col;
    size_t        len, currpos;

    len = w*h;
    currpos = 0;
    colorarr = calloc(len, sizeof(Color));
    if (!colorarr)
        return 1; /* memory error */

    /* The image data is composed of bytes representing every pixel in 
     * the image. The pixels in turn are represented of red, green and
     * blue values. If there are 4 channels, there's an alpha value, too,
     * and must be taken in consideration. */
    
    data_end = imgdata + w*h*ch;    /* get end of image_data */
    col.alpha = 0xFF;               /* default value for alpha */

    while(imgdata < data_end) {     /* read data and get color values */
        col.red = *imgdata++;
        col.green = *imgdata++;
        col.blue = *imgdata++;
        if (ch == 4)
            col.alpha = *imgdata++;
        
        if (checkdup(colorarr, currpos, col))
            continue;
        colorarr[currpos++] = color_colordup(&col);
    }
    
    for (size_t i = 0; i < currpos; i++) {  /* print and delete */
        color_print(colorarr[i]);
        free(colorarr[i]);
    }
    free(colorarr);

    return 0;
}

int checkdup(Color **arr, size_t currpos, Color c)
{
    for (int i = 0; i < currpos; i++) {
        if (color_colorcmp(&c, arr[i]))
            return 1;   /* found */
    }
    return 0;   /* not found */
}

