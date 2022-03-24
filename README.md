Bare Metal Programming on Raspberry Pi 3
========================================

Hello there! This tutorial series are made for those who would like to compile their own bare metal application
for the Raspberry Pi.

The target audience is hobby OS developers, who are new to this hardware. I'll give you examples on how to do the
basic things, like writing to the serial console, reading keystrokes from it, setting screen resolution and draw to
the linear frame buffer. I'm also going to show you how to get the hardware's serial number, a hardware-backed random
number, and how to read files from the boot partition.

This is *not* a tutorial on how to write an OS. I won't cover topics like memory management and virtual file systems,
or how to implement multi-tasking. If you plan to write your own OS for the Raspberry Pi, I suggest to do some
research before you continue. This tutorial is strickly about interfacing with the hardware, and not about OS theory.
For that, I'd recommend [raspberry-pi-os](https://github.com/s-matyukevich/raspberry-pi-os).

I assume you have a fair GNU/Linux knowledge on how to compile programs and create disk and file system images. I
won't cover those in detail, although I'll give you a few hints about how to set up a cross-compiler for this architecture.

Why Raspberry Pi 3?
-------------------

I've choosen this board for several reasons: first of all, it's cheap and easy to get. Second, it's a 64 bit
machine. I gave up programming for 32 bit long long time ago. The 64 bit is so much more interesting, as it's
address space is increadibly huge, bigger than the storage capacity which allows us to use some interesting new
solutions. Third, uses only MMIO which makes it easy to program.

For 32 bit tutorials, I'd recommend:

[Cambridge tutorials](http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/) (ASM and 32 bit only),

[David Welch's tutorials](https://github.com/dwelch67/raspberrypi) (mostly C, with some 64 bit examples),

[Peter Lemon's tutorials](https://github.com/PeterLemon/RaspberryPi) (ASM only, also for 64 bit) and

[Leon de Boer's tutorials](https://github.com/LdB-ECM/Raspberry-Pi) (C and ASM, also for 64 bit, more complex examples like USB and OpenGL).

What about Raspberry Pi 4?
--------------------------

More or less the same hw with the same peripherals, only the MMIO_BASE address is different (as long as this tutorial concerned,
most of the differences are irrelevant for us). For RPi4 tutorials I'd recommend

[rpi4-osdev tutorials](https://isometimes.github.io/rpi4-osdev)

Why not C++?
------------

The C language in "freestanding" mode allows us to develop directly to the hardware. With C++ this is not
possible, because that requires a runtime library. If you are interested in this, then I suggest to take a look
at the brilliant [Circle C++](https://github.com/rsta2/circle) library, which not only contains the mandatory
C++ runtime, but also implements every Raspberry Pi functionalities we're about to discuss in these tutorials (and even more).

Why not Rust?
-------------

Simply because my personal opinion is that Rust is a much higher level language than preferable for bare metal, something like
with C++. But if you provide the required runtime libraries, you can do it. My multiplatform system boot loader has an
[example Rust kernel](https://gitlab.com/bztsrc/bootboot) too and @andre-richter ported these tutorials to Rust.
He has added considerably more code to his [Rust repository](https://github.com/rust-embedded/rust-raspi3-OS-tutorials) which
is a very good start if you're interested in this language.

A few questions popped up on [ycombinator](https://news.ycombinator.com/item?id=24637129) regarding this. First, please note
that I've started the sentance with "my personal opinion" (and I mean Rust has a considerably larger and more complex grammar
than C, and it's easy to forget with cargo that you actually must compile in all library dependencies). Second, and please
don't get this the wrong way, but if you find clicking on the "Rust port" link too complicated then maybe low-level
programming is not the best hobby for you!

Prerequisites
-------------

Before you can start, you'll need a cross-compiler (see 00_crosscompiler directory for details)
and a Micro SD card with [firmware files](https://github.com/raspberrypi/firmware/tree/master/boot) on a FAT filesystem.

Every directory has a Makefile.gcc and a Makefile.clang. Make sure the Makefile symlink points to the version according to
the cross-compiler of your choosing.  I'd like to say thanks to [@laroche](https://github.com/laroche) for testing these
tutorials for the first time with Clang too.

I recommend to get a [Micro SD card USB adapter](http://media.kingston.com/images/products/prodReader-FCR-MRG2-img.jpg)
(many manufacturers ship SD cards with such an adapter), so that you can connect the card to any desktop computer just
like an USB stick, no special card reader interface required (although many laptops have those these days). If you dislike
`dd`, then for writing images I recommend [USBImager](https://gitlab.com/bztsrc/usbimager) which is simple GUI app with a
portable executable available for Windows, MacOSX and Linux.

You can create an MBR partitioning scheme on the SD card with an LBA FAT32 (type 0x0C) partition, format it
and copy *bootcode.bin*, *start.elf* and *fixup.dat* onto it. Or alternatively you can download a raspbian image,
`dd` it to the SD card, mount it and delete the unnecessary .img files. Whichever you prefer. What's important, you'll
create `kernel8.img` with these tutorials which must be copied to the root directory on the SD card, and no other `.img`
files should exists there.

I'd also recommend to get an [USB serial debug cable](https://www.adafruit.com/product/954). You connect it to the
GPIO pins 14/15, and run minicom on your desktop computer like

```sh
minicom -b 115200 -D /dev/ttyUSB0
```

Emulation
---------

Unfortunately official qemu binary does not support Raspberry Pi 3 yet. But good news, I've implemented that, so
it's coming soon (UPDATE: available in [qemu 2.12](https://wiki.qemu.org/ChangeLog/2.12#ARM)). Until then, you have to
compile qemu from the latest source. Once compiled, you can use it with:

```sh
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio
```

Or (with the file system tutorials)

```sh
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -drive file=$(yourimagefile),if=sd,format=raw -serial stdio
```

**-M raspi3b**
The first argument tells qemu to emulate Raspberry Pi 3 Model B hardware.

**-kernel kernel8.img**
The second tells the kernel filename to be used.

**-drive file=$(yourimagefile),if=sd,format=raw**
In second case this argument tells the SD card image too, which can be a standard rasbian image as well.

**-serial stdio**

**-serial null -serial stdio**
Finally the last argument redirects the emulated UART0 to the standard input/output of the terminal running qemu, so that everything
sent to the serial line will be displayed, and every key typed in the terminal will be received by the vm. Only works with the
tutorials 05 and above, as UART1 is *not* redirected by default. For that, you would have to add something like
`-chardev socket,host=localhost,port=1111,id=aux -serial chardev:aux` (thanks [@godmar](https://github.com/godmar) for the info),
or simply use two `-serial` arguments (thanks [@cirosantilli](https://github.com/cirosantilli)).

**!!!WARNING!!!** Qemu emulation is rudimentary, only the most common peripherals are emulated! **!!!WARNING!!!**

About the hardware
------------------

There are lots of pages on the internet describing the Raspberry Pi 3 hardware in detail, so I'll be brief and
cover only the basics.

The board is shipped with a [BCM2837 SoC](https://github.com/raspberrypi/documentation/tree/master/hardware/raspberrypi/bcm2837) chip.
That includes a

 - VideoCore GPU
 - ARM-Cortex-A53 CPU (ARMv8)
 - Some MMIO mapped peripherals.

Interestingly the CPU is not the main processor on the board. When it's powered up, first GPU runs. When it's
finished with the initialization by executing the code in bootcode.bin, it will load and execute the start.elf executable.
That's not an ARM executable, but compiled for the GPU. What interests us is that start.elf looks for different
ARM executables, all starting with `kernel` and ending in `.img`. As we're going to program the CPU in AArch64 mode,
we'll need `kernel8.img` only, which is the last to look for. Once it's loaded, the GPU triggers the reset line on
the ARM processor, which starts executing code at address 0x80000 (or more precisely at 0, but the GPU puts an [ARM
initialisation and jump code](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S) there first).

The RAM (1G for the Raspberry Pi 3) is shared among the CPU and the GPU, meaning one can read what the other has
written into memory. To avoid confusion, a well defined, so called [mailbox interface](https://github.com/raspberrypi/firmware/wiki/Mailboxes)
is established. The CPU writes a message into the mailbox, and tells the GPU to read it. The GPU (knowing that the
message is entirely in memory) interprets it, and places a response message at the same address. The CPU has
to poll the memory to know when the GPU is finished, and then it can read the response.

Similarily, all peripherals communicates in memory with the CPU. Each has it's dedicated memory address starting from
0x3F000000, but it's not in real RAM (called Memory Mapped IO). Now there's no mailbox for peripherals, instead each
device has it's own protocol. What's common for these devices that their memory must be read and written in 32 bit
units at 4 bytes aligned addresses (so called words), and each has control/status and data words. Unfortunately
Broadcom (the manufacturer of the SoC chip) is legendary bad at documenting their products. The best we've got is the
BCM2835 documentation, which is close enough.

There's also a Memory Management Unit in the CPU which allows creating virtual address spaces. This can be programmed
by specific CPU registers, and care must be taken when you map these MMIO addresses into a virtual address space.

Some of the more interesting MMIO addresses are:
```
0x3F003000 - System Timer
0x3F00B000 - Interrupt controller
0x3F00B880 - VideoCore mailbox
0x3F100000 - Power management
0x3F104000 - Random Number Generator
0x3F200000 - General Purpose IO controller
0x3F201000 - UART0 (serial port, PL011)
0x3F215000 - UART1 (serial port, AUX mini UART)
0x3F300000 - External Mass Media Controller (SD card reader)
0x3F980000 - Universal Serial Bus controller
```
For more information, see Raspberry Pi firmware wiki and documentation on github.

https://github.com/raspberrypi

Good luck and enjoy hacking with your Raspberry! :-)

bzt
