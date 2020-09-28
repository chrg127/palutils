/* *******************************************************************
 *                          colorutils.h
 * A library providing simple functions for working with colors.
 *
 * *******************************************************************/

#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <stdint.h>

typedef union {
    uint32_t value;
    struct {
        uint8_t red, green, blue, alpha;
    };
} Color;

int     color_strtocolor(char *s, Color *c);
void    color_formatcolor(char *s);
int     color_compare(const void *c1, const void *c2);
Color  *color_dup(Color c);

#endif

