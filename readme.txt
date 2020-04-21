--- palutils ---

A small collection of utilities for working with palettes. This was 
mostly a hobby project to learn how to work with libpng, so the code
quality may not be the best.

List of binaries:

getcolorvals        - A small programs that find color values in files.
                      Assumes color values start with '#'. Good luck
                      finding a worthy use case for this. I made this 
                      mostly to work efficiently with vim color configs.

getpal              - Extracts a palette from images and prints each
                      color in the palette to screen. Useful if you don't
                      wanna open your image editor (or if your image
                      editor is shit).
                      
makepal             - Given a list of color values, constructs an image.
                      A good way to use this is to use getpal to get the
                      values first, then use makepal to build the palette
                      image. Or if you have fun creating images by writing
                      hexadecimal values.

List of files:

colorutils.c        - A small library for working with colors. Kinda shit.
colorutils.h
readpng.c           - A library for reading PNG files. Abstracts a part of libpng.
readpng.h
writepng.c          - A library for writing PNG files. Also abstracts a part of libpng.
writepng.h
getcolorvals.c
getpal.c
makepal.c
test/               - For testing the binaries.


--- Compiling ---

Make sure to install libpng, of course. On Linux, it's already installed.
On Windows, it's best if you use Mingw64 (which I also used).
The Makefile should be able to compile everything without errors. By default,
it will statically link libpng and zlib. If you don't want this, open the
Makefile and change (line 7):
    LIBS = $(STLIBS)
to:
    LIBS = $(SHLIBS)

