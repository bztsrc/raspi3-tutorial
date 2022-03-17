A Raspberry Pi 3 alacsony szintű programozása
=============================================

Üdvözöllek! Ez az oktatóanyag azok számára készült, akik szeretnének saját alacsony szintű (bare metal)
programot írni a Raspberry Pi-jükre.

A célközönsége elsősorban azok a hobby OS fejlesztők, akik most ismerkednek ezzel a hardverrel. Példákat mutatok
arra, hogyan kell az alapfunkciókat megvalósítani, úgy mint soros vonalra írás, billentyűleütés kiolvasás, beállítani
a képernyő felbontását és rajzolni rá. Azt is megmutatom, hogy kell az alaplap sorszámát és igazi, vas által
generált véletlenszámot kiolvasni, valamint hogy hogyan kell fájlokat beolvasni a kártyáról.

Ez az oktatóanyag *nem* arról szól, hogy hogyan írjunk operációs rendszert. Nem érint olyan témákat, mint a
memória gazdálkodás, virtuális fájlrendszer kezelés, vagy hogy hogyan valósítsuk meg a multitaszkot. Ha saját
OS-t szeretnél írni a Raspberry Pi-re, akkor javaslom előbb nézz körbe máshol, mielőtt folytatnád. Ez az
oktatóanyag kifejezetten arról szól, hogy kommunikáljunk az adott hardverrel, és nem az operációs rendszerek
hátteréről. Ha ez érdekel, ezt javaslom: [raspberry-pi-os](https://github.com/s-matyukevich/raspberry-pi-os).

Feltételezem, hogy elegendő GNU/Linux ismerettel rendelkezel, tudod, hogy kell programokat fordítani és lemez
valamint fájlrendszer képfájlokat létrehozni. Nem fogok kitérni ezekre, bár pár apró tanácsot adok arról, hogy
hogyan állítsuk be a kereszt-fordítót kifejezetten ehhez az architektúrához.

Miért Raspberry Pi 3?
---------------------

Több okból is erre esett a választásom: először is olcsó, könnyen beszerezhető. Másodszor teljesen 64 bites
masina. Már réges rég felhagytam a 32 bites fejlesztéssel, mivel a 64 bit sokkal izgalmasabb. A címterülete
hatalmas, nagyobb, mint a tárolókapacitás, ezért lehetővé teszi új, izgalmas megoldások létrehozását.
Harmadsorban MMIO-t használ, amit könnyű programozni.

32 bites oktatóanyagokhoz a következőket ajánlom:

[Cambridge-i oktatóanyagok](http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/) (ASM és csak 32 bites példák),

[David Welch oktatóanyagai](https://github.com/dwelch67/raspberrypi) (főleg C, néhány 64 bites példával),

[Peter Lemon oktatóanyagai](https://github.com/PeterLemon/RaspberryPi) (csak ASM, 64 bites példák is) és

[Leon de Boer oktatóanyagai](https://github.com/LdB-ECM/Raspberry-Pi) (C és ASM, 64 bites és összetettebb példák is, mint USB és OpenGL).

Mi a helyzet Raspberry Pi 4 fronton?
------------------------------------

Többé-kevésbé ugyanaz a vas, és ugyanazok a perifériák csak az MMIO_BASE címe más (mármint ami ezt az oktatóanyagot érinti,
a legtöbb különbség minket most nem érint). RPi4 oktatóanyagnak a következőt ajánlom:

[rpi4-osdev oktatóanyagai](https://isometimes.github.io/rpi4-osdev)


Miért nem C++?
--------------

A C nyelv "freestanding" módban lehetővé teszi, hogy direktben a vasra programozzunk. C++ esetében ez nem lehetséges,
ott mindenképp szükség van egy runtime függvénykönyvtárra. Ha valakit ez érdekel, akkor ajánlom figyelmébe a
nagyszerű [Circle C++](https://github.com/rsta2/circle) függvénykönyvtárat, ami nemcsak a C++-hoz szükséges függvényeket
tartalmazza, hanem minden Raspberry Pi funkció, amit ebben az oktatóanyagban sorra veszünk (és még több is) már eleve
implementálva van benne, csak meg kell hívni őket.

Miért nem Rust?
---------------

Egyszerűen azért, mert a személyes véleményem az, hogy a Rust nyelv felsőbb szintű, mint ami ideális a direkt vas programozásához,
valahogy úgy, ahogy C++ esetében. De ha biztosítod a szükséges runtime függvénykönyvtárat, akkor lehetséges. A multiplatform
rendszerbetöltőmhöz van egy [minta Rust kernel](https://gitlab.com/bztsrc/bootboot) is, és @andre-richter portolta ezeket az
oktatóanyagokat Rustra. Később jelentősen átírta és kibővítette a [Rust repó](https://github.com/rust-embedded/rust-raspi3-OS-tutorials)ját,
ami kiváló kiindulás ha ez a programozási nyelv érdekel.

Előkészületek
-------------

Mielőtt belevágnánk, szükséged lesz egy kereszt-fordítóra (lásd 00_crosscompiler könyvtár) és egy Micro SD
kártyára néhány [firmware fájllal](https://github.com/raspberrypi/firmware/tree/master/boot) egy FAT partíción.

Minden könyvtárban található egy Makefile.gcc és egy Makefile.clang. Bizonyosodj meg róla, hogy a Makefile symlink az általad
választott kereszt-fordítódra mutat-e. Szeretném külön megköszönni [@laroche](https://github.com/laroche)-nak, amiért az
oktatóanyagokat elsőként Clang-al is letesztelte.

Javaslom, hogy szerezz be egy [Micro SD kártya USB adaptert](http://media.kingston.com/images/products/prodReader-FCR-MRG2-img.jpg)
(sok gyártó eleve szállít ilyent az SD kártyáihoz), hogy könnyedén csatlakoztathasd az asztali gépedhez, mint egy
pent, speciális kártya olvasó interfész nélkül (habár sok laptopban gyárilag van ilyen olvasó manapság). Ha nem szereted a `dd`-t,
akkor a lemezképek írásához javaslom az [USBImager](https://gitlab.com/bztsrc/usbimager)t, ami egy egyszerű GUI-s alkalmazás, nem
igényel telepítést és elérhető Windows, MacOSX és Linux rendszerekhez.

Az MBR partíciós táblát létre kell hozni az SD kártyán LBA FAT32 (0x0C típusú) partícióval, leformázni azt,
majd rámásolni a *bootcode.bin*, *start.elf* és *fixup.dat* állományokat. Alternatívaként letöltheted a raspbian képfájlt,
`dd`-vel rárakhatod a kártyára, majd mountolás után letörölheted a felesleges .img fájlokat. Amenyik szimpatikusabb. A lényeg
az, hogy ezekben az oktatóanyagokban `kernel8.img` fájlokat gyártunk, amit a partíció gyökér könyvtárába kell másolni,
és más `.img` kiterjesztésű fájl nem lehet ott.

Javaslom továbbá, hogy vegyél egy [soros USB debug kábelt](https://www.adafruit.com/product/954). Ezt a GPIO 14-es
és 15-ös lábára kell csatlakoztatni, az asztali gépen pedig a következő paranccsal kell elindítani a minicom-ot:

```sh
minicom -b 115200 -D /dev/ttyUSB0
```

Emulálás
--------

Sajnálatos módon a hivatalos qemu bináris nem támogatja a Raspberry Pi 3-at egyenlőre. De van egy jó hírem, megírtam
a támogatást hozzá, így hamarosan érkezik (FRISSÍTÉS: elérhető a [qemu 2.12](https://wiki.qemu.org/ChangeLog/2.12#ARM)-ben). Addig
sajnos fordítani kell a qemu-t a legfrissebb forrásból. Miután lefordult, így tudod használni:

```sh
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio
```

Vagy (a fájl rendszer oktatóanyagok esetében)

```sh
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -drive file=$(yourimagefile),if=sd,format=raw -serial stdio
```

**-M raspi3b**
Az első paraméter utasítja a qemu-t a Raspberry Pi 3 hardver emulálására.

**-kernel kernel8.img**
A második megadja a használandó kernel fájl nevét.

**-drive file=$(yourimagefile),if=sd,format=raw**
A második esetben az extra paraméter megadja az SD kártya képfájl nevét, ami lehet egy sima raspbian képfájl is akár.

**-serial stdio**

**-serial null -serial stdio**
Végezetül az utolsó paraméter lehetővé teszi, hogy az emulált gép UART0-ját átirányítsuk a qemu-t futtató terminál be- és
kimenetére, azaz hogy minden, a virtuális gépen soros vonalra küldött karater megjelenjen a terminálon, az ott leütött karaktereket
pedig kiolvashassuk a vm-en. Ez csak az 5-ös oktatóanyagtól működik, mivel az UART1 alapból *nem* irányítódik át. Ehhez plusz
paraméterekre van szükség, mint például `-chardev socket,host=localhost,port=1111,id=aux -serial chardev:aux` (köszönet
[@godmar](https://github.com/godmar)-nak az infóért), vagy egyszerűen kétszer kell megadni a `-serial` kapcsolót (köszönet
[@cirosantilli](https://github.com/cirosantilli)).

**!!!FIGYELEM!!!** Qemu emulálása felületes, csak a legáltalánosabb perifériákat támogatja! **!!!FIGYELEM!!!**

Magáról a hardverről
--------------------

Rengeteg oldal foglalkozik az interneten a Raspberry Pi 3 hardverének részletes bemutatásával, szóval itt rövid
leszek, és csak az alapokat futjuk át.

Az alaplapon egy [BCM2837 SoC](https://github.com/raspberrypi/documentation/tree/master/hardware/raspberrypi/bcm2837) csip
található. Ebbe beszereltek

 - VideoCore grafikus processzort (GPU)
 - ARM-Cortex-A53 általános processzort (CPU, ARMv8)
 - És néhány MMIO-val leképzett perifériát.

Érdekesség, hogy a nem a CPU a fő processzor. Amikor bekapcsoljuk, először a GPU kezd futni. Amikor végzett az
inicializálásával, ami a bootcode.bin futtatását jelenti, betölti a start.elf programot. Ez nem egy ARM-es futtatható
állomány, hanem a GPU-ra íródott. Ami számunkra érdekes, az az, hogy ez a start.elf különféle ARM futtathatók után
kutat, melyek mind `kernel`-el kezdődnek, és `.img`-re végződnek. Mivel mi a CPU-t AArch64 módban fogjuk programozni,
ezért nekünk a `kernel8.img` fájlra van szükségünk, ami a legutolsó keresett fájlnév. Miután betöltötte, a GPU magasba
emeli az ARM processzor reset lábát, aminek hatására elkezdi a futtatást a 0x80000-as címen (egész pontosan a 0-ás
címen, csak oda a GPU egy [ARM inicializáló kódot és ugrás utasítást](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S) rakott előtte).

A RAM (1G a Raspberry Pi 3-on) meg van osztva a CPU és a GPU között, ezért az egyik tudja olvasni, amit a másik
a memóriába írt. A félreértések elkerülése végett egy jól definiált, úgy nevezett levelesláda [mailbox](https://github.com/raspberrypi/firmware/wiki/Mailboxes)
interfészt alakítottak ki. A CPU beletesz egy üzenetet a levesládába, és szól a GPU-nak, hogy üzenete van. A GPU
(tudván, hogy a teljes üzenet a memóriában van) értelmezi azt, és ugyanarra a címre egy választ rak. A CPU-nak
folyamatosan figyelni kell a memóriát, hogy végzett-e a GPU, és ha igen, akkor és csak akkor kiolvashatja a választ.

Hasonlóan a perifáriák is a memórián keresztül kommunikálnak a CPU-val. Mindegyiknek saját dedikált címe van,
0x3F000000-tól kezdődően, de ez nem igazi RAM (MMIO, memóriába leképzett ki- és bemenet). Namost itt nincs levelesláda,
minden eszköz saját protokollt beszél. Ami közös, az az, hogy ez a memóriarész csak 32 bites adagokban, 4-el osztható
címen írható / olvasható (szavak), és mindegyiknek kontroll/státusz illetve adat szavai vannak. Sajnálatos módon
a Broadcom (a SoC gyártója) hírhedten szarul dokumentája a termékeit. A legjobb, ami van, a BCM2835-ös leírása, ami
azért eléggé hasonló.

Van továbbá lapcímfordító egység (MMU) a CPU-ban ami lehetővé teszi virtuális címterek használatát. Ez néhány
speciális CPU rendszer regiszterrel programozható, és oda kell figyelni, amikor ezeket az MMIO területeket képezük le
vele a virtuális címtérbe.

Néhány az érdekesebb MMIO címek közül:
```
0x3F003000 - Rendszer Időzítő (System Timer)
0x3F00B000 - Megszakítás vezérlő (Interrupt controller)
0x3F00B880 - VideoCore levelesláda (VideoCore mailbox)
0x3F100000 - Energiagazdálkodás (Power management)
0x3F104000 - Véletlenszám generátor (Random Number Generator)
0x3F200000 - Általános célú ki- és bemenet vezérlő (General Purpose IO controller)
0x3F201000 - UART0 (soros port, PL011)
0x3F215000 - UART1 (soros port, AUX mini UART)
0x3F300000 - Külső tároló vezérlő (External Mass Media Controller, SD kártya olvasás)
0x3F980000 - USB vezérlő (Universal Serial Bus controller)
```
A többi információ megtalálható a Raspberry Pi firmware wiki-ben és a documentation repóban a github-on.

https://github.com/raspberrypi

Sok szerencsét és élvezetes hekkelést a Raspberry-dhez! :-)

bzt
