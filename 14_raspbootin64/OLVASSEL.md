Oktatóanyag 14 - Raspbootin64
=============================

Mivel folyton iragatni az SD kártyát strapás, és nem tesz jót a kártyának, ezért egy olyan kernel8.img-t készítünk,
ami a soros vonalról fogja betölteni az igazi kernel8.img-t.

Ez az oktatóanyag a jól ismert [raspbootin](https://github.com/mrvn/raspbootin) átírása 64 bitre.
A betöltőprogram egyik felét adja csak, a kernel fogadót, ami az RPi-n fut. A másik fél, a PC-n futó küldő,
megtalálható az eredeti forrásban [raspbootcom](https://github.com/mrvn/raspbootin/blob/master/raspbootcom/raspbootcom.cc) néven.
Ha Windows vagy MacOSX gépekről is szeretnél kernelt küldeni, vagy jobban csak szereted a GUI alkalmazásokat, akkor javaslom
inkább az [USBImager](https://gitlab.com/bztsrc/usbimager) programot a `-S` (nagybetű) kapcsolóval. Ezen kvül,
[@milanvidakovic](https://github.com/milanvidakovic) volt olyan jó fej, hogy megosztotta az általa írt
[Java-s kernel küldő](https://github.com/milanvidakovic/Raspbootin64Client)t (ne zavarjon a név, igazából ez a szerver oldali
rész, de a név lényegtelen is).

Hogy az új kernelt ugyanoda tölthessük be, el kell mozdítanunk a kódunkat az útból. Ezt chain loading-nak hívják, amikor
az első kód ugyanarra a címre tölti be a második kódot, ezért az utóbbi azt hiszi, a firmware töltötte be.
Hogy ezt megvalósítsuk, egy alacsonyabb címre linkeljük a kódot, és mivel a GPU ettől függetlenül a 0x80000-ra tölt be,
nekünk kell a módosított címre másolnunk magunkat. Amikor végeztünk, a 0x80000-as címen lévő memóriának használaton
kívülinek kell lennie.

Ajánlott a kódunkat minimalizálni, mivel úgyis figyelmen kívül hagyja az újonnan betöltendő kód. Ezért kivettem az
`uart_puts()` és még néhány eljárást, így sikerült a teljes méretet 1024 bájt alá csökkenteni. Így biztos, hogy a
0x80000 - 1024 megfelelő link cím lesz, ahol a kódunk nem nyúlik bele a load címbe. Ezt a következő paranccsal ellenőrizheted:

```sh
$ aarch64-elf-readelf -s kernel8.elf | grep __bss_end
    27: 000000000007ffb0     0 NOTYPE  GLOBAL DEFAULT    4 __bss_end
```

Start
-----

Először is elmentjük a regiszter paramétereket. Hozzáadtam egy ciklust, ami átmásolja a kódunkat arra a címre, ahová
vártuk, hogy betöltődjön. Végül meghívjuk a relokált C kódot. Mivel a gcc RIP-relatív ugrást generál, nekünk kell a
relokálást hozzáadni a címhez.

Köszönet [@mrvn](https://github.com/mrvn)-nek, amiért felhívta a figyelmem a nem relokált végtelen ciklusra. Szerencsére
a firmver változott, és csak a BSP-n fut ez a kód, ezért senkinek nem okozott gondot és senkinek nem is tűnt fel.

Linker
------

Másik címre linkelünk ebben a példában. Hasonlóan a bss méret kiszámításához, ugyanúgy meghatározzuk a kód
méretét is, amit másolnunk kell.

Main
----

Kiírjuk, hogy 'RBIN64', majd beolvassuk az új kernelt a soros vonalról, pontosan oda, ahová a start.elf is töltötte volna.
Majd visszaállítjuk a regiszterparamétereket és átadjuk a vezérlést az új kernelre abszolút címzést használva.
