/* *******************************************************************
 *                          colorutils.h
 * A library providing simple functions for working with colors.
 *
 * *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "colorutils.h"


int color_iscolor(char *s)
{
    int i, gothash;

    i = gothash = 0;
    if (s[i] == '#') {  /* ignore leading hash tag */
        ++i;
        ++gothash;
    }
    while (isxdigit(s[i])) /* check if every character is a hex digit */
        ++i;

    if (s[i] != '\0')
        return 0;
    if (i-gothash != 6 && i-gothash != 8) /* alpha or no alpha */
        return 0;

    return 1;
}

int color_colorcmp(Color *c1, Color *c2)
{
    return c1->red  == c2->red  && c1->green == c2->green &&
           c1->blue == c2->blue && c1->alpha == c2->alpha;
}

void color_strtocolor(char *s, Color *c)
{
    color_strtoval(s, &(c->red), &(c->green), &(c->blue), &(c->alpha));
}

void color_strtoval(char *s, unsigned char *red, unsigned char *green, 
        unsigned char *blue, unsigned char *alpha)
{
    char v[3]; /* container for strings of single values */
    int len = strlen(s);
    
    v[2] = '\0';
    if (s[0] == '#') {  /* check for leading '#' */
        ++s;
        --len;
    }
    strncpy(v, s, 2); /* extract the singles values */
    *red = color_htoi(v);
    strncpy(v, s+2, 2);
    *green = color_htoi(v);
    strncpy(v, s+4, 2);
    *blue = color_htoi(v);
    if (len == 8) {         /* check for alpha */
        strncpy(v, s+6, 2);
        *alpha = color_htoi(v);
    } else
        *alpha = 0xFF;
}

char * color_formatcolor(char *s)
{
    int i;

    if (s[0] == '#')
        memmove(s, s+1, strlen(s));
    i = 0;
    while (s[i] != '\0') {
        if (s[i] > 'a' && s[i] < 'f')
            s[i] = toupper(s[i]);
    }
    return s;
}

Color * color_colordup(Color *c)
{
    if (!c)
        return NULL;
    Color *new = malloc(sizeof(Color));
    if (!new)
        return NULL;
    new->red = c->red;
    new->green = c->green;
    new->blue = c->blue;
    new->alpha = c->alpha;
    return new;
}

void color_print(Color *c)
{
    printf("%02X%02X%02X%02X\n", c->red, c->green, c->blue, c->alpha);
}

void color_getvals(Color *c, unsigned char *red, unsigned char *green, 
        unsigned char *blue, unsigned char *alpha)
{
    *red = c->red;
    *green = c->green;
    *blue = c->blue;
    *alpha = c->alpha;
}


int color_htoi(char *hptr)
{
    int i, n;
    
    for (i = 0; hptr[i] == ' '; ++i) /* skip all trailing spaces */
        ;
    if (hptr[i] == '0' && toupper(hptr[i+1]) == 'X') /* skip '0x' part */
        i += 2;
    n = 0;
    for ( ; isxdigit(hptr[i]); ++i) /* convert till it finds an invalid character */
        n = 16 * n + (color_hctoi(toupper(hptr[i])));

    return n;
}

int color_hctoi(char c)
{
    if (isdigit(c))
        return c - '0';
    return c - 55;
}

