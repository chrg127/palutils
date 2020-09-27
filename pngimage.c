#include "pngimage.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <zlib.h>

int pngimage_read_init(Image *img, FILE *infile)
{
    unsigned char sig[8];

    if (!img || !infile)
        return IMAGE_ERR_BADPARAM;
    
    /* read signature */
    fread(sig, 1, 8, infile);
    if (!png_check_sig(sig, 8))
        return IMAGE_ERR_NOTIMAGE;
    
    /* create png data structs */
    img->pngdata = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!img->pngdata)
        return IMAGE_ERR_NOMEM;
    img->pnginfo = png_create_info_struct(img->pngdata);
    if (!img->pnginfo) {
        png_destroy_read_struct(&img->pngdata, NULL, NULL);
        return IMAGE_ERR_NOMEM;
    }
    
    /* this is libpng's error handling: must be put into each func that calls
     * a libpng func */
    if (setjmp(png_jmpbuf(img->pngdata))) {
        png_destroy_read_struct(&img->pngdata, &img->pnginfo, NULL);
        return IMAGE_ERR_GENERIC;
    }

    /* initialize and read IHDR chunk: it contains infos such as width and
     * height */
    png_init_io(img->pngdata, infile);
    png_set_sig_bytes(img->pngdata, 8);
    png_read_info(img->pngdata, img->pnginfo);
    png_get_IHDR(img->pngdata, img->pnginfo, &img->w, &img->h, &img->bitdepth, &img->colortype, NULL, NULL, NULL);
    return 0;
}

int pngimage_read_fillimgdata(Image *img)
{
    uint32_t i, rowbytes;
    uint8_t *rowpointers[img->h];

    if (setjmp(png_jmpbuf(img->pngdata))) {
        png_destroy_read_struct(&img->pngdata, &img->pnginfo, NULL);
        return IMAGE_ERR_GENERIC;
    }
    
    /* transform the image so that we will always get data in rgba form */
    if (img->colortype == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(img->pngdata);
    if (img->colortype == PNG_COLOR_TYPE_GRAY && img->bitdepth < 8)
        png_set_expand(img->pngdata);
    if (png_get_valid(img->pngdata, img->pnginfo, PNG_INFO_tRNS))
        png_set_expand(img->pngdata);
    if (img->bitdepth == 16)
        png_set_strip_16(img->pngdata);
    if (img->colortype == PNG_COLOR_TYPE_GRAY || img->colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(img->pngdata);
    
    /* update transformations */
    png_read_update_info(img->pngdata, img->pnginfo);

    /* get channels and alloc raw image data */
    rowbytes = png_get_rowbytes(img->pngdata, img->pnginfo);
    img->ch = (int) png_get_channels(img->pngdata, img->pnginfo);
    img->data = malloc(rowbytes*img->h);
    if (!img->data) {
        png_destroy_read_struct(&img->pngdata, &img->pnginfo, NULL);
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
    png_read_image(img->pngdata, rowpointers);
    png_read_end(img->pngdata, NULL);
    return 0;
}

void pngimage_read_cleanup(Image *img)
{
    if (img->data)
        free(img->data);
    if (img->pngdata && img->pnginfo)
        png_destroy_read_struct(&img->pngdata, &img->pnginfo, NULL);
    img->data = NULL;
    img->pngdata = NULL;
    img->pnginfo = NULL;
}

