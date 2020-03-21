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

static mainprog_info maininfo;

int readcolors(FILE *);

int main(int argc, char **argv)
{
    FILE *infile, *image_file;
    int i, retval, exitval;
    char image_name[] = "palette.png";
    unsigned char r, g, b, a;
    
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [LIST FILE]\n", *argv);
        return 1; 
    }

    exitval = 0;
    if (colorlist_init() == 2)
        goto memerr;

    infile = image_file = NULL;
    if (argc == 1) {  /* read from stdin */
        retval = readcolors(stdin);
        if (retval == 1)
            goto formaterr;
        if (retval == 2)
            goto memerr;
    } else if (argc == 2) { /* read from file */
        infile = fopen(argv[1], "r");
        if (!infile) {
            fprintf(stderr, "ERROR: couldn't open file %s\n", argv[1]);
            goto defaulterr;
        }
        retval = readcolors(infile);
        if (retval == 1)
            goto formaterr;
        if (retval == 2)
            goto memerr;
    }
    
#ifdef DEBUG
    colorlist_print();
    printf("Size: %ld\n", colorlist_size());
#endif

    image_file = fopen(image_name, "wb");
    if (!image_file) {
        fprintf(stderr, "ERROR: couldn't create image file\n");
        goto defaulterr;
    }

    /* set up the simplest possible mainprog_info: we want only an IHDR chunk, an IDAT chunk and an IEND chunk */
    maininfo.width        = colorlist_size();
    maininfo.height       = 1;
    maininfo.outfile      = image_file;
    maininfo.filter       = 0;
    maininfo.sample_depth = 8;
    maininfo.interlaced   = 0;
    maininfo.have_bg      = 0; //1;
    //maininfo.bg_red     = 0xFF;
    //maininfo.bg_green   = 0xFF;
    //maininfo.bg_blue    = 0xFF;
    maininfo.have_time  = 0;
    maininfo.have_text  = 0;
    maininfo.gamma      = 0.0;

    retval = writepng_init(&maininfo, PNG_COLOR_TYPE_RGBA);
    if (retval == 2)
        goto libpngerr;
    if (retval == 4)
        goto memerr;

    maininfo.image_data = malloc(colorlist_size()*4*sizeof(char));
    if (!maininfo.image_data)
        goto memerr;
        
    colorlist_rewind();
    i = 0;
    while (colorlist_getnext(&r, &g, &b, &a) != 1) {
        maininfo.image_data[i++] = r;
        maininfo.image_data[i++] = g;
        maininfo.image_data[i++] = b;
        maininfo.image_data[i++] = a;
    }
    /* due to the nature of this program, we just need to set up one row.
     * to get an image with more than one row, you'd have to do another loop,
     * one which loops over each row */
    /* get image data for one row and call writepng_encode_row() */
    if (writepng_encode_row(&maininfo) == 2)
        goto libpngerr;

    if (writepng_encode_finish(&maininfo) == 2)
        goto libpngerr;

    printf("Wrote list to %s file\n", image_name);
    goto cleanup;

/* yes, there are a lot labels for error handling... but this avoids the issue of writing error messages everywhere... */
memerr:
    fputs("ERROR: out of memory\n", stderr);
    goto defaulterr;
formaterr:
    fputs("ERROR: list format error\n", stderr);
    goto defaulterr;
libpngerr:
    fputs("ERROR: libpng error\n", stderr);
    goto defaulterr;
defaulterr:
    ++exitval;
    goto cleanup;
    
cleanup:
    if (infile)
        fclose(infile);
    if (image_file)
        fclose(image_file);
    if (maininfo.image_data)
        free(maininfo.image_data);
    colorlist_free();
    writepng_cleanup(&maininfo);
    return exitval;
}

/* readcolors: get the color values from a list file
 * returns 0 on success, 1 on list format error, 2 on memory error */
int readcolors(FILE *infile)
{
    char *s;
    int linen, len;
    size_t n;
    
    linen = 0;
    s = NULL;
    while((len = getline(&s, &n, infile)) != -1) {
        ++linen;

        if (s[len-1] == '\n') /* remove newline */
            s[len-1] = '\0';

        if (!iscolor(s))
            return 1;

        if (colorlist_insert_str(s) == 2) /* 1 is for already in list. it can be ignored. */
            return 2;
    }
    free(s);

    return 0;
}

