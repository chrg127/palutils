#ifndef PNGIMAGE_H_INCLUDED
#define PNGIMAGE_H_INCLUDED

#include <png.h>
#include <stdint.h>

typedef struct _image {
    unsigned char *data;
    uint32_t w, h;
    png_structp pngdata;
    png_infop   pnginfo;
    int ch, bitdepth, colortype;
} Image;

enum {
    IMAGE_ERR_GENERIC = 1,
    IMAGE_ERR_BADPARAM,
    IMAGE_ERR_NOMEM,
    IMAGE_ERR_NOTIMAGE,
};

int     pngimage_read_image(Image *img, FILE *infile);
void    pngimage_read_cleanup(Image *img);
int     pngimage_write_image_rgba(Image *img, FILE *outfile);
void    pngimage_write_cleanup(Image *img);

#endif

