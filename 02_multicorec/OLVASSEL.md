Oktatóanyag 02 - Többmagos C
============================

Próbáljunk valami összetettebbet, mit szóltok? Összetettebb alatt azt értem, hogy most is megállítjuk a CPU magokat,
akárcsak az első oktatóanyagban, de most az egyik magot C-ből!

Start
-----

Most már meg kell különböztetnünk a magokat. Ehhez kiolvassuk a *mpidr_el1* rendszer regisztert. Ha nem nulla, akkor
a korábbi végtelen ciklus következik. Ha nulla, akkor meg fogunk hívni egy C eljárást. De ehhez előbb be állítanunk
egy megfelelő vermet, ki kell nulláznunk a bss szegmenst mielőtt kiadhatnánk az ugrás parancsot. Hozzáadtam néhány
Assembly sort, amik mindezt elvégzik. Arra az esetre, ha a C eljárás visszatérne (nem szabadna), ugyanarra a
végtelen ciklusra ugrunk, mint amit a többi CPU mag is épp végrehajt.

FIGYELEM: a firmver verziójától függően lehetséges, hogy az alkalmazás magok le vannak állítva. Ha ez az eset áll fenn,
akkor a kódunk eleve csak a 0-ás magon fut, de nem okoz bajt, ha mégis lecsekkoljuk a mag számát. A többi mag elindításához
a futtatandó funkció címét kell beírni a 0xE0, 0xE8, 0xF0 címekre (rendre minden maghoz egy cím). Lásd [armstub8.S](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S#L129).

Makefile
--------

Egy kisit trükkösebb lett. Hozzáadtam parancsokat a C fordításhoz, de akkor már általánosan. Mostantól ugyanazt
a Makefile-t használhatjuk minden oktatóanyaghoz, függetlenül attól, hány C forrásfájlunk van, és a továbbiakban
nem is szerepeltetem.

Linker script
-------------

Hasonlóan, a linker szkript is bonyolultabbá vált, mivel a C-hez adat és bss szekciókra is szükség van. Hozzáadtam
továbbá egy számolást a bss szegmens méretének megállapítására, így egyszerűen hivatkozhatunk rá Assembly-ben, és
nem kell ott molyolni vele.

Fontos, hogy a text szegmens az Assembly kóddal kezdődjön, mivel ez elé raktuk a vermet, ezért kell a KEEP().
Íly módon mind a betöltési címünk 0x80000, akárcsak a `_start` cimke címe és a verem teteje.

Main
----

Végezetül, az első C kódunk. Csak egy végtelen ciklus, de akkor is! :-)
