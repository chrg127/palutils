#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "colorlist.h"

#define CVALLEN 8   /* lenght in characters of a color value */

/* function prototypes */
void getvalues(FILE *f);
int collectvalue(FILE *f);
int ishexdigit(char c);

int main(int argc, char **argv)
{
    FILE *f;
    
    if ((colorlist_init()) == 1) {
        fprintf(stderr, "ERROR: Out of memory\n");
        return 1;
    }

    if (argc == 1) { /* no arguments: get values from stdin */
        getvalues(stdin);
    } else {
        while (--argc) {
            f = fopen(*++argv, "r");
            if (!f) {
                fprintf(stderr, "ERROR: Couldn't open file %s\n", *argv);
                continue;
            }
            getvalues(f);
            fclose(f);
        }
    }
    
    colorlist_print();

    colorlist_free();
    return 0;
}

void getvalues(FILE *f)
{
    int c;

    while ((c = getc(f)) != EOF) /* get values */
        if (c == '#')
            collectvalue(f);
}

/* collectvalue: collect a color value and insert into color value array
 * returns 1 on error, 0 on success
 */
int collectvalue(FILE *f)
{
    char valuestr[CVALLEN+1]; /* +1 for terminating character */
    int i, c, value;
    
    i = 0;
    while (ishexdigit(c = getc(f)) && i != CVALLEN) /* collect value */
        valuestr[i++] = toupper(c);
    if (i != CVALLEN && i != CVALLEN-2) /* a color value is either 6 or 8 characters */
        return 1;
    valuestr[i] = '\0';
    
    if (colorlist_checkdup(valuestr) == 0) { /* insert the value if a copy hasn't been inserted already */
        if (colorlist_insert(valuestr) == 1) {
            fputs("ERROR: Out of memory\n", stderr);
            exit(1);
        }
    }
    else
        return 1;
    return 0;
}

/* ishexdigit: check if a character is a valid hex digit. */
int ishexdigit(char c)
{
    if (isdigit(c))
        return 1;
    switch (toupper(c))
    {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

