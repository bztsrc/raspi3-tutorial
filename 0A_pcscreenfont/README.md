Tutorial 0A - PC Screen Font
============================

Drawing pixmaps is fun, but definitely there's a need to display characters as well. Basicaly fonts
are nothing more than bitmaps for each character. For this tutorial I choosed PC Screen Font format,
the same Linux Console uses.

UPDATE: because I had many requests, I've added demonstration on how to print UTF-8 strings.

Lfb.h, lfb.c
------------

`lfb_init()` sets up resolution, depth, and color channel order. Also queries framebuffer's address.

`lfb_print(x,y,s)` displays a string on screen with fixed-sized glyphs using PSF.

`lfb_proprint(x,y,s)` displays a string on screen with proportional SSFN font.

Font.psf
--------

The font file. Use any file from /usr/share/kbd/consolefonts. Unicode table is not supported. Translating
characters to glyph index using that table (instead of one-to-one relation) is a homework for you. This font
is generated from the original IBM PC VGA 8x16 Font ROM, and includes 127 glyphs.

Font.sfn
--------

One of the biggest drawbacks of PSF that it does not store glyph metrics. To support UTF-8 strings, you will
need proportional fonts (that is, 8x16 bitmaps for Latin script and 16x16 for CJK glyphs for example). So I've
added a demonstration on how to use [Scalable Screen Font](https://gitlab.com/bztsrc/scalable-font2) to
overcome this. More information, example fonts and font converter can be found in the SSFN repository.

Makefile
--------

I've added two object files, generated from the psf and sfn fonts. It's a good example of how to
include and reference a binary file in C. I've used the following command to find out the label:

```sh
$ aarch64-elf-readelf -s font_psf.o
        ... output removed for clarity ...
     2: 0000000000000820     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_end
     3: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_start
     4: 0000000000000820     0 NOTYPE  GLOBAL DEFAULT  ABS _binary_font_psf_size
```

Main
----

Very simple. We set the resolution and display the string. First with PSF, and then with SSFN.
