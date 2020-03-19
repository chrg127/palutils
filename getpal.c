/* **********************************************************
 *                      getpal.c
 * A very simple program which takes one or more images and
 * prints its palette to stdout.
 * The palette is a list of colors where every element is 
 * unique.
 * Only PNG images are supported.
 * 
 * **********************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

/*#define DEBUG*/

#include "readpng.h"
#include "colorlist.h"


void  getcolors(FILE *img, char *fname);
void  insertcolor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
char *btoa(unsigned char b, char *s, int);

int main(int argc, char **argv)
{
    FILE *image_file;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [image files...]\n", *argv);
        return 1;
    }
    if (colorlist_init() == 1) {
        fputs("ERROR: Out of memory\n", stderr);
        return 1;
    }

    while (--argc > 0) {    /* parse arguments */
        ++argv;
        image_file = fopen(*argv, "rb");
        if (!image_file) {
            fprintf(stderr, "ERROR: Can't read %s\n", argv[1]);
            continue;
        }
        getcolors(image_file, *argv);
        fclose(image_file);
    }

    colorlist_print();
    colorlist_free();

    return 0;
}

/* getcolors: read the PNG image file and extract its colors */
void getcolors(FILE *img, char *fname)
{
    int           err, channels;
    unsigned long width, height, rowbytes;
    unsigned char bgred, bggreen, bgblue, red, green, blue, alpha, *image_data, *data_end;
    
    /* init gets the width and height of the PNG file and does a few error checks */
    err = readpng_init(img, &width, &height);
    switch(err) {
    case 1:
        fprintf(stderr, "ERROR: %s: Not a PNG file\n", fname);
        goto cleanup;
    case 2:
        fprintf(stderr, "ERROR: %s: Corrupted PNG file\n", fname);
        goto cleanup;
    case 4:
        fputs("ERROR: Out of memory\n", stderr);
        exit(1);
    default: /* no error */
        break;
    }
    
    /* the image data is composed of bytes representing every pixel in the image.
     * the pixels in turn are represented of red, green and blue values.
     * if there are 4 channels, there's an alpha value, too, and must be taken in consideration. */
    image_data = readpng_get_image(&channels, &rowbytes); /* get image data */
    if (!image_data) {
        fputs("ERROR: Out of memory\n", stderr);
        exit(1);
    }
    data_end = image_data + width*height*channels;        /* find where it ends */

    alpha = 0xFF;   /* with no alpha channel, the alpha always remains at FF */
    while(image_data < data_end) {  /* read data and get color values */
        red = *image_data++;
        green = *image_data++;
        blue = *image_data++;
        if (channels == 4)
            alpha = *image_data++;
        insertcolor(red, green, blue, alpha);
    }
    readpng_cleanup(1); /* finished reading data, do cleanup */

cleanup:
    readpng_cleanup(0);
}

/* insertcolor: insert a color value into the array color_list */
void insertcolor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    char value[9], tmp[3]; /* strings of 8 and 2 chars + 1 for terminator */

    btoa(r, value, 2);
    strncat(value, btoa(g, tmp, 2), 2);
    strncat(value, btoa(b, tmp, 2), 2);
    strncat(value, btoa(a, tmp, 2), 2);

#ifdef DEBUG
    printf("insertcolor: %hhx %hhx %hhx %hhx %s\n", r, g, b, a, value);
#endif
    
    if (colorlist_checkdup(value) == 0) {   /* check if value is already in the list */
        if (colorlist_insert(value) == 1) { /* and insert it */
            fputs("ERROR: Out of memory\n", stderr);
            exit(1);
        }
    }
}

/* btoa: convert byte b to string. assume s is big enough. */
char *btoa(unsigned char b, char *s, int padding)
{
    static char hextable[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    int i, j, tmp, len;
    
    i = 0;
    do {
        s[i++] = hextable[b%16]; /* convert */
    } while((b /= 16) != 0);
    for ( ; i < padding; i++)
        s[i] = '0';
    s[i] = '\0';
    len = i-1;
    for (i = 0, j = len; i < j; i++, j--) { /* reverse string */
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
    return s;
}

