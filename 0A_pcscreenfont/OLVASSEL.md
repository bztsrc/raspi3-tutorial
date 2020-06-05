Oktatóanyag 0A - PC Screen Font
===============================

Képeket kirakni poénos, de mindenképpen szükség van többre, karakterek megjelenítésére is. Alapvetően
a fontok nem mások, mint képek minden karakterhez (glifek). Ehhez az oktatóanyaghoz azt a PC Screen Font
formátumot választottam, amit a Linux Console csomag is használ.

FRISSÍTÉS: mivel sok megkeresést kaptam ez ügyben, ezért azt is bemutatom, hogy kell UTF-8 sztringet kiírni.

Lfb.h, lfb.c
------------

`lfb_init()` beállítja a felbontást, mélységet, színcsatornákat és visszaadja a framebuffer címét.

`lfb_print(x,y,s)` megjelenít egy szöveget a képernyőn fix méretű glifeket tartalmazó PSF-el.

`lfb_proprint(x,y,s)` megjelenít egy szöveget a képernyőn proporcionális SSFN fonttal.

Font.psf
--------

A font fájl. Bármelyik használható a /usr/share/kbd/consolefonts mappából. Unicode táblákat nem támogatja.
A karakterek glifnek való megfeleltetése ezen táblázat által (a meglévő egy-az-egyhez megfeleltetés helyett)
házi feladat, Rád van bízva. Ezt a fontot az eredeti IBM PC VGA ROM-jában található 8x16 fontkészletből generáltam,
és 127 glyphet tartalmaz.

Font.sfn
--------

A PSF egyik legnagyobb hátránya, hogy nem tárol glif méreteket. Hogy UTF-8 sztringeket írhassunk ki, szükség van
proporcionális fontokra (ami például 8x16-os bitmapben tárolja a Latin betűket, de 16x16-osban a kínai és egyéb
ázsiai glifet). Ezért bemutatom azt is, hogyan kell [Scalable Screen Font](https://gitlab.com/bztsrc/scalable-font2)
formátumot használni, hogy megoldjuk ezt a problémát. Bővebb információt, példa fontokat és font konvertálót az SSFN
repóban találsz.

Makefile
--------

Két új object-et adtam hozzá, amik a psf és sfn fájlokból generálódnak. Jó példa arra, hogyan kell bináris fájlt
behúzni és hivatkozni C forrásból. A következő parancsot használtam a cimke nevének kiderítésére:

```sh
$ aarch64-elf-readelf -s font_psf.o
        ... kimenet törölve az átláthatóság miatt ...
     2: 0000000000000820     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_end
     3: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_start
     4: 0000000000000820     0 NOTYPE  GLOBAL DEFAULT  ABS _binary_font_psf_size
```

Main
----

Nagyon egyszerű. Beállítjuk a felbontást és megjelenítjük a szöveget. Először PSF-et használva, aztán SSFN-t.
