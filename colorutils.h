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

int colorlist_init(void);

int colorlist_checkdup(Color *);
int colorlist_insert(Color *);
int colorlist_insert_str(char *);
int colorlist_insert_vals(unsigned char, unsigned char, unsigned char, unsigned char);

int colorlist_getnext(unsigned char *, unsigned char *, unsigned char *, unsigned char *);

void colorlist_print(void);
void colorlist_free(void);

int iscolor(char *);
char *format_color(char *);
void strtocval(char *, unsigned char *, unsigned char *, unsigned char *, unsigned char *);
int htoi(char *hptr);
int hctoi(char c);
int ishexdigit(char c);

void strtocolor(char *, Color *);
int colorcmp(Color *, Color *);

#endif

