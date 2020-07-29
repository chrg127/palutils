/* *******************************************************************
 *                          colorutils.h
 * A library providing simple functions for working with colors.
 *
 * *******************************************************************/

#ifndef COLORUTILS_H
#define COLORUTILS_H

//#include <stdint.h>

typedef union {
    unsigned int value;
    struct {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
    };
} Color;

#define COLOR_MEMERR 2

/* Checks if s is a valid color, i. e. it's a hex value of 6 or 8 
 * characters, with either upper and lower characters and with an optional 
 * leading '#'. */
int     color_iscolor(char *s);
/* Compares two colors. returns 1 if they're equal, 0 if not */
int     color_colorcmp(Color *c1, Color *c2);
/* Converts a string to a color. Assumes s is a valid color. */
void    color_strtocolor(char *s, Color *c);
/* Gets the color values given a valid string color. */
void    color_strtoval(char *s, unsigned char *r, unsigned char *g, 
                unsigned char *b, unsigned char *a);
/* Formats a color string like so: RRGGBBAA. 
 * Assumes it is a valid color value. */
char *  color_format_color(char *s);
/* Copies a color into a safe space. 
 * The duplicated color must be later freed. */
Color * color_colordup(Color *c);
/* Prints a color. */
void color_print(Color *c);
/* Gets the color values from a color. */
void color_getvals(Color *c, unsigned char *red, unsigned char *green, 
        unsigned char *blue, unsigned char *alpha);

/* Some hex functions... */
/* Converts hptr string to an int. Returns the converted number. */
int     color_htoi(char *hptr);
/* Converts a single hex character to int. Assumes the character 
 * is an hex digit. Returns the converted number. */
int     color_hctoi(char c);
/* Check if a character is a valid hex digit. 
 * Returns 1 if it is, 0 if not. */
int     color_ishexdigit(char c);


#endif

