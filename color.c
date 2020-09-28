/* *******************************************************************
 *                          colorutils.h
 * A library providing simple functions for working with colors.
 *
 * *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "color.h"


static int __iscolorstr(char *s)
{
    int i = 0, h = 0;

    /* ignore leading hash tag */
    if (s[i] == '#')
        i = h = 1;
    /* check if every character is a hex digit */
    while (isxdigit(s[i]))
        ++i;
    if (s[i] != '\0' || (i-h != 6 && i-h != 8))
        return 0;
    return 1;
}

static int __hctoi(char c)
{
    if (isdigit(c))
        return c - '0';
    return c - 55;
}


static int __htoi(char *hptr)
{
    int i, n;
    
    for (i = 0; hptr[i] == ' '; ++i) /* skip all trailing spaces */
        ;
    if (hptr[i] == '0' && toupper(hptr[i+1]) == 'X') /* skip '0x' part */
        i += 2;
    n = 0;
    for ( ; isxdigit(hptr[i]); ++i) /* convert till it finds an invalid character */
        n = 16 * n + (__hctoi(toupper(hptr[i])));

    return n;
}

int color_strtocolor(char *s, Color *c)
{
    int val;
    
    if (!s || !c)
        return 1;
    if (__iscolorstr(s) == 0)
        return 1;
    if (s[0] == '#')
        s++;
    val = __htoi(s);
    c->value = val;
    return 0;
}

void color_formatcolor(char *s)
{
    int i;

    if (s[0] == '#')
        memmove(s, s+1, strlen(s));
    for (i = 0; s[i]; i++)
        if (s[i] > 'a' && s[i] < 'f')
            s[i] = toupper(s[i]);
}

int color_compare(const void *c1, const void *c2)
{
    const Color *col1 = (const Color *) c1;
    const Color *col2 = (const Color *) c2;
    if (col1->value < col2->value)
        return -1;
    else if (col1->value > col2->value)
        return 1;
    return 0;
}

Color *color_dup(Color c)
{
    Color *cptr = malloc(sizeof(Color));
    if (!cptr)
        return NULL;
    cptr->value = c.value;
    return cptr;
}

