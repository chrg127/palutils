/* **************************************************************
 *                      colorutils.c
 * A library providing simple functions for working with colors.
 * It also provides a way to create and manage a list of colors
 * (by using the colorlist_ tagged functions
 *
 * **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "colorutils.h"

static unsigned long    _listlenght = 100;  /* self explanatory */
static Color          **_colorlist;         /* used for storing colors */
size_t                  _currpos = 0;       /* points to first free pos */

/* colorlist_init: initializes the color list. call this before doing operations on the list. */
int colorlist_init(void)
{
    _colorlist = calloc(_listlenght, sizeof(Color));
    if (!_colorlist)
        return 1;
    return 0;
}

/* colorlist_checkdup: check if color c already appears in _colorlist. returns 1 if it appears, 0 if not */
int colorlist_checkdup(Color *c)
{
    for (int i = 0; i < _currpos; i++) {
        if (colorcmp(c, _colorlist[i]))
            return 1;
    }
    return 0;
}

/* colorlist_insert_str: insert a color into the list. returns 1 if already into list, 2 for memory error */
int colorlist_insert(Color *c)
{
    if (colorlist_checkdup(c)) {  /* find if it's already in the list */
        free(c);
        return 1;
    }
    
    if (_currpos == _listlenght) {  /* increase list size if necessary */
        _listlenght *= 2;
        _colorlist = realloc(_colorlist, _listlenght);
        if (!_colorlist)
            return 2;
    }

    _colorlist[_currpos++] = c;     /* insert */
    return 0;
}

/* the user can also insert via strings and single values with these two functions: */

/* this function assumes s is a valid color. call iscolor to check first! */
int colorlist_insert_str(char *s)
{
    Color *c;
    int retval;
    
    c = malloc(sizeof(Color));
    if (!c)
        return 2;
    strtocolor(s, c);
    retval = colorlist_insert(c);
    return retval;
}

int colorlist_insert_vals(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    Color *c;

    c = malloc(sizeof(Color));
    if (!c)
        return 2;
    c->red = r;
    c->green = g;
    c->blue = b;
    c->alpha = a;
    return colorlist_insert(c);
}

/* colorlist_getnext: get the next element from color list. returns 1 if at the end of list */
int colorlist_getnext(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
    Color *c = _colorlist[_currpos];
    if (!c)
        return 1;
    ++_currpos;
    *r = c->red;
    *g = c->green;
    *b = c->blue;
    *a = c->alpha;
    return 0;
}

/* colorlist_print: print all the colors in the list */
void colorlist_print(void)
{
    for (int i = 0; i < _currpos; i++)
        printf("%02X%02X%02X%02X\n", _colorlist[i]->red, _colorlist[i]->green, _colorlist[i]->blue, _colorlist[i]->alpha);
}

/* colorlist_free: free the _colorlist array. */
void colorlist_free(void)
{
    for (int i = 0; i < _currpos; i++)
        free(_colorlist[i]);
    free(_colorlist);
}

/* iscolor: check if a string is a valid color. a valid color is a hex value with both upper and 
 * lower characters, with an optional leading '#'. no other characters are allowed. */
int iscolor(char *s)
{
    int i, gothash;

    i = gothash = 0;
    if (s[i] == '#') {  /* ignore leading hash tag */
        ++i;
        ++gothash;
    }
    while (ishexdigit(s[i]))
        ++i;

    if (s[i] != '\0')
        return 0;
    if (i-gothash != 6 && i-gothash != 8) /* alpha or no alpha */
        return 0;

    return 1;
}

/* format_color: formats a color string like so: RRGGBBAA. assumes it is a valid color value. */
char * format_color(char *s)
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

/* strtocval: given a valid string color, get the color values. */
void strtocval(char *color, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha)
{
    char tmp[3];
    int len = strlen(color);
    
    if (color[0] == '#') { /* check for leading '#' */
        ++color;
        --len;
    }
    tmp[2] = '\0';
    strncpy(tmp, color, 2);
    *red = htoi(tmp);
    color += 2;
    strncpy(tmp, color, 2);
    *green = htoi(tmp);
    color += 2;
    strncpy(tmp, color, 2);
    *blue = htoi(tmp);
    if (len == 8) { /* check for alpha */
        color += 2;
        strncpy(tmp, color, 2);
        *alpha = htoi(tmp);
    }
}

/* htoi: convert hptr string to an int */
int htoi(char *hptr)
{
    int i, n;
    
    for (i = 0; hptr[i] == ' '; ++i) /* skip all trailing spaces */
        ;
    if (hptr[i] == '0' && toupper(hptr[i+1]) == 'X') /* skip '0x' part */
        i += 2;
    n = 0;
    for ( ; ishexdigit(hptr[i]); ++i) /* convert till it finds an invalid character */
        n = 16 * n + (hctoi(toupper(hptr[i])));

    return n;
}

/* hctoi: convert a single hex character to int. assumes the character is an hex digit. */
int hctoi(char c)
{
    if (isdigit(c))
        return c - '0';
    return c - 55;
}

/* ishexdigit: check if a character is a valid hex digit. */
int ishexdigit(char c)
{
    if (isdigit(c)) /* check for numbers */
        return 1;
    switch (toupper(c)) {
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': /* check for letters */
        return 1;
    default:
        return 0;
    }
}

/* strtocolor: convert a string to a color. assumes s is a valid color. */
void strtocolor(char *s, Color *c)
{
    char v[3];
    int len;
    
    v[2] = '\0';
    len = strlen(s);

    if (s[0] == '#') {
        ++s;
        --len;
    }
    strncpy(v, s, 2);
    c->red = (unsigned char) htoi(v);
    strncpy(v, s+2, 2);
    c->green = (unsigned char) htoi(v);
    strncpy(v, s+4, 2);
    c->blue = (unsigned char) htoi(v);
    if (len == 8) {
        strncpy(v, s+4, 2);
        c->alpha = (unsigned char) htoi(v);
    } else
        c->alpha = 0xFF;
}

/* colorcmp: compare two colors */
int colorcmp(Color *c1, Color *c2)
{
    if (c1->red != c2->red)
        return 0;
    if (c1->green != c2->green)
        return 0;
    if (c1->blue != c2->blue)
        return 0;
    if (c1->alpha != c2->alpha)
        return 0;
    return 1;
}
