Tutorial 15 - Write Sector
==========================

Let's go back to tutorial 0B for a moment, and add a function to write the SD card. To test it, we'll implement
a boot counter. We're going to read a sector into memory (same as in 0B), increase a counter in the sector buffer,
then save it back to the SD card. This way every time we reboot this image, the counter should be increased.

For the counter I've choosen the last 4 bytes of the 2nd sector. I did not want to use the first sector, as that
could mess up the Master Boot Record and render our card unbootable. Second sector is safer, although if you're
using an EFI Partitioning Table (like I do), then the counter could mess up that too. So I've choosen the last 4 bytes
of the sector hoping the table is shorter than 508 bytes. If this not the case for you, then change the COUNTER_SECTOR
define to point to a surely unused sector on your SD card.

I'd would like to say thanks to [@DamianOslebo](https://github.com/DamianOslebo) for thgroughful testing and spotting
a bad constant in the command defines.

Sd.h, sd.c
------------

I've added new commands (CMD_WRITE_SINGLE, CMD_WRITE_MULTI) and new status flags (SR_WRITE_AVAILABLE,
INT_WRITE_RDY) to our driver. We'll use them instead of their *READ* counterparts.

`sd_writeblock(buffer,lba,num)` write num blocks (sectors) from the buffer into the SD card starting at sector lba.

Main
----

We read a block after the bss segment in memory, and increase a counter in it, and save it back to the card. If
everything went well, we print the actual counter value on the console.
