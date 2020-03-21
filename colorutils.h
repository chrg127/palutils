/* header file for colorutils.c */

#ifndef COLORUTILS_H
#define COLORUTILS_H

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
} Color;

extern size_t _currpos;
#define colorlist_size() (_currpos)
#define colorlist_rewind() (_currpos = 0)

/* for error handling, 1 is a function specific error, while 2 usually means a memory error */

/* initializes the color list. call this before doing operations on the list. returns 2 if out of memory. */
int colorlist_init(void);

/* check if a color already appears in the list. returns 1 if it appears, 0 if not */
int colorlist_checkdup(Color *);

/* inserts a color into the list. returns 1 if already into list, 2 for memory error */
int colorlist_insert(Color *);
/* a version using a string as input. it assumes the string is a valid color. use iscolor() first. */
int colorlist_insert_str(char *);
/* a version using single values for each channel. */
int colorlist_insert_vals(unsigned char, unsigned char, unsigned char, unsigned char);

/* gets the next element from color list. returns 1 if at the end of list */
int colorlist_getnext(unsigned char *, unsigned char *, unsigned char *, unsigned char *);

/* prints all the colors in the list */
void colorlist_print(void);

/* frees the list */
void colorlist_free(void);


/* checks if a string is a valid color. a valid color is a hex value of 6 or 8 characters 
 * with both upper and lower characters, with an optional leading '#'. no other characters are allowed. */
int iscolor(char *);
/* compares two colors. returns 1 if they're equal, 0 if not */
int colorcmp(Color *, Color *);
/* converts a string to a color. assumes s is a valid color. */
void strtocolor(char *, Color *);
/* gets the color values given a valid string color. */
void strtocval(char *, unsigned char *, unsigned char *, unsigned char *, unsigned char *);
/* formats a color string like so: RRGGBBAA. assumes it is a valid color value. */
char *format_color(char *);


/* converts hptr string to an int. returns the converted number. */
int htoi(char *hptr);
/* converts a single hex character to int. assumes the character is an hex digit. returns the converted number. */
int hctoi(char c);
/* check if a character is a valid hex digit. returns 1 if it is, 0 if not. */
int ishexdigit(char c);


#endif

