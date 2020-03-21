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

int readcolors(FILE * infile, int interactive);
int iscolor(char *s);
int ishexdigit(char c);

int main(int argc, char **argv)
{
    FILE *infile, *image;
    unsigned char r, g, b, a;
    int i;
    char image_name[] = "palette.png";
    
    colorlist_init();
    infile = image = NULL;

    if (argc == 1) {  /* read from stdin */
        if (readcolors(stdin, 1) != 0)
            goto cleanup;
    } else if (argc == 2) { /* read from file */
        infile = fopen(argv[1], "r");
        if (!infile) {
            fprintf(stderr, "ERROR: couldn't open file %s\n", argv[1]);
            goto cleanup;
        }
        if (readcolors(infile, 0) != 0)
            goto cleanup;
    } else {
        fprintf(stderr, "Usage: %s [LIST FILE]\n", *argv);
        goto cleanup;
    }
    
#ifdef DEBUG
    colorlist_print();
    printf("Size: %ld\n", colorlist_size());
#endif

    image = fopen(image_name, "wb");
    if (!image) {
        fprintf(stderr, "ERROR: couldn't create output image\n");
        goto cleanup;
    }

    /* set up the simplest possible mainprog_info: we want only an IHDR chunk, an IDAT chunk and an IEND chunk */
    maininfo.width        = colorlist_size();
    maininfo.height       = 1;
    maininfo.outfile      = image;
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

    writepng_init(&maininfo, PNG_COLOR_TYPE_RGBA);
    
    maininfo.image_data = malloc(colorlist_size()*4*sizeof(char));
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
    writepng_encode_row(&maininfo);
    writepng_encode_finish(&maininfo);
    free(maininfo.image_data);
    printf("Wrote list to %s file\n", image_name);

cleanup:
    if (infile)
        fclose(infile);
    if (image)
        fclose(image);
    colorlist_free();
    writepng_cleanup(&maininfo);
    return 0;
}

/* readcolors: get the color values from a list file
 * returns 0 on success, 1 on list format error, 2 on memory error */
int readcolors(FILE *infile, int interactive)
{
    char *s;
    int linen, len;
    size_t n;
    
    linen = 0;
    s = NULL;
    while((len = getline(&s, &n, infile)) != -1) {
        ++linen;

        if (interactive && s[0] == '\n')
            break;  /* in interaction mode, a line with nothing in it will end interaction */
        
        if (s[len-1] == '\n') /* remove newline */
            s[len-1] = '\0';

        if (!iscolor(s)) {
            fprintf(stderr, "ERROR: list format error at line %d\n", linen);
            return 1;
        }

        if (colorlist_insert_str(s) == 2) { /* 1 is for already in list. it can be ignored. */
            fprintf(stderr, "ERROR: out of memory\n");
            return 2;
        }
    }
    free(s);

    return 0;
}

