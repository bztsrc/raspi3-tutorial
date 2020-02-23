Oktatóanyag 15 - Szektor Kiírás
===============================

Térjünk egy kicsit vissza a 0B oktatóanyaghoz, és egészítsük ki az SD kártya írással. Hogy leteszteljük,
egy újraindítás számlálót fogunk készíteni. Beolvassunk egy szektort (akárcsak a 0B-ben), növelünk egy
számlálót a bufferben, majd visszaírjuk a kártyára. Így minden alkalommal, mikor újraindítjuk a gépet,
a számláló eggyel növekedni fog.

A számlálóhoz a második szektor utolsó 4 bájtját választottam. Nem akartam az első szektort írni, mivel
az esetleg elronthatja a Master Boot Record-ot, indíthatatlanná téve a kártyánkat. A második szektor
biztonságosabb, habár ha EFI Partíciós Táblát használsz (mint én), akkor a számláló tönkreteheti azt is.
Ezért a legutolsó 4 bájtot választottam, remélve, hogy a tábla rövidebb, mint 508 bájt. Ha ez nem állja
meg a helyét, akkor változtasd meg a COUNTER_SECTOR értékét, hogy egy biztosan használaton kívüli szektorra
mutasson.

Szeretném külön megköszönni [@DamianOslebo](https://github.com/DamianOslebo) segítségét az alapos tesztelésért
és amiért kiszúrt egy hibás parancs definíciót.

Sd.h, sd.c
------------

Új parancsokat (CMD_WRITE_SINGLE, CMD_WRITE_MULTI) és új státuszokat (SR_WRITE_AVAILABLE, INT_WRITE_RDY)
adtam hozzá a meghajtóprogramunkhoz. Ezeket fogjuk használni a *READ* megfelelőik helyett.

`sd_writeblock(buffer,lba,num)` kiír egy blokkot (szektort) a bufferből az SD kártyára az lba-tól kezdve.

Main
----

A blokkot a bss szegmens utánra töltjük be, növeljük a számlálót, majd vissza kiírjuk a kártyára. Ha minden
rendben ment, akkor a számláló aktuális értékét kiírjuk a soros konzolra.
