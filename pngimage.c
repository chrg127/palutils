#include "pngimage.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <zlib.h>
#include <setjmp.h>

int pngimage_read_image(Image *img, FILE *infile)
{
    unsigned char sig[8];
    png_structp data;
    png_infop info;
    uint32_t i, rowbytes;

    if (!img || !infile)
        return IMAGE_ERR_BADPARAM;

    /* read signature */
    fread(sig, 1, 8, infile);
    if (!png_check_sig(sig, 8))
        return IMAGE_ERR_NOTIMAGE;

    /* create png data structs */
    data = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!data)
        return IMAGE_ERR_NOMEM;
    info = png_create_info_struct(data);
    if (!info) {
        png_destroy_read_struct(&data, NULL, NULL);
        return IMAGE_ERR_NOMEM;
    }

    /* this is libpng's error handling: must be put into each func that calls
     * a libpng func */
    if (setjmp(png_jmpbuf(data))) {
        png_destroy_read_struct(&data, &info, NULL);
        return IMAGE_ERR_GENERIC;
    }

    /* initialize and read IHDR chunk: it contains infos such as width and
     * height */
    png_init_io(data, infile);
    png_set_sig_bytes(data, 8);
    png_read_info(data, info);
    png_get_IHDR(data, info, &img->w, &img->h, &img->bitdepth, &img->colortype, NULL, NULL, NULL);

    uint8_t *rowpointers[img->h];

    /* transform the image so that we will always get data in rgba form */
    if (img->colortype == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(data);
    if (img->colortype == PNG_COLOR_TYPE_GRAY && img->bitdepth < 8)
        png_set_expand(data);
    if (png_get_valid(data, info, PNG_INFO_tRNS))
        png_set_expand(data);
    if (img->bitdepth == 16)
        png_set_strip_16(data);
    if (img->colortype == PNG_COLOR_TYPE_GRAY || img->colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(data);
    png_read_update_info(data, info);

    /* get channels and alloc raw image data */
    rowbytes = png_get_rowbytes(data, info);
    img->ch = (int) png_get_channels(data, info);
    img->data = malloc(rowbytes*img->h);
    if (!img->data) {
        png_destroy_read_struct(&data, &info, NULL);
        return IMAGE_ERR_NOMEM;
    }

#ifdef DEBUG
    fprintf(stderr, "pngimage_read_fillimgdata: channels = %d, rowbytes = %d, height = %d\n",
            img->ch, rowbytes, img->h);
#endif

    /* set the individual row_pointers to point at the correct offsets */
    for (i = 0; i < img->h; i++)
        rowpointers[i] = img->data + i*rowbytes;
    /* read whole image and end */
    png_read_image(data, rowpointers);
    png_read_end(data, NULL);
    png_destroy_read_struct(&data, &info, NULL);

    return 0;
}

int pngimage_write_image_rgba(Image *img, FILE *outfile)
{
    png_structp data;
    png_infop info;

    data = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!data)
        return IMAGE_ERR_NOMEM;
    info = png_create_info_struct(data);
    if (!info) {
        png_destroy_write_struct(&data, NULL);
        return IMAGE_ERR_NOMEM;
    }

    if (setjmp(png_jmpbuf(data))) {
        png_destroy_write_struct(&data, &info);
        return IMAGE_ERR_GENERIC;
    }

    png_init_io(data, outfile);
    png_set_compression_level(data, Z_BEST_COMPRESSION);
    png_set_IHDR(data, info, img->w, img->h, 8, PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(data, info);
    png_set_packing(data);

    for (uint32_t i = 0; i < img->h; i++)
        png_write_row(data, img->data);
    png_write_end(data, NULL);
    png_destroy_write_struct(&data, &info);
    return 0;
}

