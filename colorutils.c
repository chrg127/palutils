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

static size_t           _listlenght = 100;  /* self explanatory */
static Color          **_colorlist;         /* used for storing colors */
size_t                  _currpos = 0;       /* points to first free pos */

int colorlist_init(void)
{
    _colorlist = calloc(_listlenght, sizeof(Color));
    if (!_colorlist)
        return 2;
    return 0;
}

int colorlist_checkdup(Color *c)
{
    for (int i = 0; i < _currpos; i++) {
        if (colorcmp(c, _colorlist[i]))
            return 1;   /* found */
    }
    return 0;   /* not found */
}

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

int colorlist_insert_str(char *s)
{
    Color *c;
    
    c = malloc(sizeof(Color));
    if (!c)
        return 2;
    strtocolor(s, c);
    return colorlist_insert(c);
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

void colorlist_print(void)
{
    for (int i = 0; i < _currpos; i++)
        printf("%02X%02X%02X%02X\n", _colorlist[i]->red, _colorlist[i]->green,
                                     _colorlist[i]->blue, _colorlist[i]->alpha);
}

void colorlist_free(void)
{
    for (int i = 0; i < _currpos; i++)
        free(_colorlist[i]);
    free(_colorlist);
}




int iscolor(char *s)
{
    int i, gothash;

    i = gothash = 0;
    if (s[i] == '#') {  /* ignore leading hash tag */
        ++i;
        ++gothash;
    }
    while (ishexdigit(s[i])) /* check if every character is a hex digit */
        ++i;

    if (s[i] != '\0')
        return 0;
    if (i-gothash != 6 && i-gothash != 8) /* alpha or no alpha */
        return 0;

    return 1;
}

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

void strtocolor(char *s, Color *c)
{
    strtocval(s, &(c->red), &(c->green), &(c->blue), &(c->alpha));
}

void strtocval(char *s, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha)
{
    char v[3]; /* container for strings of single values */
    int len = strlen(s);
    
    v[2] = '\0';
    if (s[0] == '#') {  /* check for leading '#' */
        ++s;
        --len;
    }
    strncpy(v, s, 2); /* extract the singles values */
    *red = htoi(v);
    strncpy(v, s+2, 2);
    *green = htoi(v);
    strncpy(v, s+4, 2);
    *blue = htoi(v);
    if (len == 8) {         /* check for alpha */
        strncpy(v, s+6, 2);
        *alpha = htoi(v);
    } else
        *alpha = 0xFF;
}

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

int hctoi(char c)
{
    if (isdigit(c))
        return c - '0';
    return c - 55;
}

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

