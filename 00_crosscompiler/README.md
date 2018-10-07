AArch64 Cross Compiler
======================

Before we could start our tutorials, you'll need some tools. Namely a compiler that compiles for the AArch64
architecture and it's companion utilities.

**IMPORTANT NOTE**: this description is not about how to compile a cross-compiler in general. It's about how to
compile one specifically for the *aarch64-elf* target. If you have problems, google "how to build a gcc cross-compiler"
or ask on an appropriate support forum for your operating system. I can't and won't help you with your environment,
you have to solve that on your own. As I've said in the introduction I assume you know how to compile programs
(including the compilation of the cross-compiler).

**NOTE**: if you don't like gcc, thanks to @laroche, the tutorials are tested with Clang too.

OS-provided cross compiler
-------------------------

Distribution derived from Debian (and possibly others) provide the `gcc-aarch64-linux-gnu` cross-compiler which works
just fine with all the code in this tutorial. By default the tutorial makefiles use the cross-compiler built according
to instructions below, but if you wish to save some time and use the one provide by your distribution you can compile
all the samples with the distribution cross-compiler using the below command line:

    make TOOLCHAIN_PREFIX=aarch64-linux-gnu

Additionally, if your distribution has clang and you have it installed, you can build all the samples with it (you **still**
need to install `gcc-aarch64-linux-gnu`) using the following command line:

    make TOOLCHAIN_PREFIX=aarch64-linux-gnu USE_CLANG=1

Build system
------------

To orchestrate compilation, we'll use GNU make. No need for cross-compiling this, as we are about to use it on the
host computer only, and not on the target machine. The reason I choosed this build system for the tutorials is that
GNU make is also required to compile the compiler, so you'll need it anyway.

Download and unpack sources
---------------------------

First of all, download binutils and gcc sources. In this example I've used the latest versions as of writing.
You are advised to check the mirrors and modify filenames accordly.

```sh
wget https://ftpmirror.gnu.org/binutils/binutils-2.30.tar.gz
wget https://ftpmirror.gnu.org/gcc/gcc-8.1.0/gcc-8.1.0.tar.gz
wget https://ftpmirror.gnu.org/mpfr/mpfr-4.0.1.tar.gz
wget https://ftpmirror.gnu.org/gmp/gmp-6.1.2.tar.bz2
wget https://ftpmirror.gnu.org/mpc/mpc-1.1.0.tar.gz
wget https://gcc.gnu.org/pub/gcc/infrastructure/isl-0.18.tar.bz2
wget https://gcc.gnu.org/pub/gcc/infrastructure/cloog-0.18.1.tar.gz
```

If you want to verify the downloads, you can also download the checksums and signatures:

```sh
wget https://ftpmirror.gnu.org/binutils/binutils-2.30.tar.gz.sig
wget https://ftpmirror.gnu.org/gcc/gcc-8.1.0/gcc-8.1.0.tar.gz.sig
wget https://ftpmirror.gnu.org/mpfr/mpfr-4.0.1.tar.gz.sig
wget https://ftpmirror.gnu.org/gmp/gmp-6.1.2.tar.bz2.sig
wget https://ftpmirror.gnu.org/mpc/mpc-1.1.0.tar.gz.sig
wget https://gcc.gnu.org/pub/gcc/infrastructure/sha512.sum
```

Once the download finished, verify the downloads by running:

```sh
sha512sum -c sha512.sum --ignore-missing
for i in *.sig; do gpg2 --auto-key-retrieve --verify-files "${i}"; done
```

The first command should say 'OK' for isl and cloog, the second should say 'Good signature' for the
other files.

Then unpack the tarballs with these commands:

```sh
for i in *.tar.gz; do tar -xzf $i; done
for i in *.tar.bz2; do tar -xjf $i; done
```

Remove the files, we don't need anymore:

```sh
rm -f *.tar.* sha512.sum
```

You'll need some symbolic links before you could start the compilation, so let's create them:

```sh
cd binutils-*
ln -s ../isl-* isl
cd ..
cd gcc-*
ln -s ../isl-* isl
ln -s ../mpfr-* mpfr
ln -s ../gmp-* gmp
ln -s ../mpc-* mpc
ln -s ../cloog-* cloog
cd ..
```

Compiling the sources
---------------------

Okay, now we have to build two packages. One is called *binutils*, which includes linker, assembler and other
useful commands.

```sh
cd binutils-*
./configure --prefix=/usr/local/cross-compiler --target=aarch64-elf \
--enable-shared --enable-threads=posix --enable-libmpx --with-system-zlib --with-isl --enable-__cxa_atexit \
--disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --disable-libssp --enable-plugin \
--disable-linker-build-id --enable-lto --enable-install-libiberty --with-linker-hash-style=gnu --with-gnu-ld\
--enable-gnu-indirect-function --disable-multilib --disable-werror --enable-checking=release --enable-default-pie \
--enable-default-ssp --enable-gnu-unique-object
make -j4
make install
```

The first argument tells the configure script to install the build in `/usr/local/cross-compiler`. The second
specifies the target architecture, for which we want the tools to be compiled. The other arguments turn specific
options on and off, don't bother. It's enough to know they are appropriately tweeked for an embedded system compiler.

And the second package, of course we'll need the *gcc compiler* itself.

```sh
cd gcc-*
./configure --prefix=/usr/local/cross-compiler --target=aarch64-elf --enable-languages=c \
--enable-shared --enable-threads=posix --enable-libmpx --with-system-zlib --with-isl --enable-__cxa_atexit \
--disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --disable-libssp --enable-plugin \
--disable-linker-build-id --enable-lto --enable-install-libiberty --with-linker-hash-style=gnu --with-gnu-ld\
--enable-gnu-indirect-function --disable-multilib --disable-werror --enable-checking=release --enable-default-pie \
--enable-default-ssp --enable-gnu-unique-object
make -j4 all-gcc
make install-gcc
```

Here we specify the same directory and architecture as before. We also tell to compile only the C compiler, as gcc
otherwise would support tons of languages we don't need. This reduces the compilation time significantly. The remaining
arguments are the same as with binutils.

Now check `bin` folder in your `/usr/local/cross-compiler` directory. You should see a lot of executables there. You
probably also want to add this directory to your PATH environment variable.

```sh
$ ls /usr/local/cross-compiler/bin
aarch64-elf-addr2line  aarch64-elf-elfedit    aarch64-elf-gcc-ranlib  aarch64-elf-ld       aarch64-elf-ranlib
aarch64-elf-ar         aarch64-elf-gcc        aarch64-elf-gcov        aarch64-elf-ld.bfd   aarch64-elf-readelf
aarch64-elf-as         aarch64-elf-gcc-7.2.0  aarch64-elf-gcov-dump   aarch64-elf-nm       aarch64-elf-size
aarch64-elf-c++filt    aarch64-elf-gcc-ar     aarch64-elf-gcov-tool   aarch64-elf-objcopy  aarch64-elf-strings
aarch64-elf-cpp        aarch64-elf-gcc-nm     aarch64-elf-gprof       aarch64-elf-objdump  aarch64-elf-strip
```

The executables we are interested in:
 - aarch64-elf-as - the assembler
 - aarch64-elf-gcc - the C compiler
 - aarch64-elf-ld - the linker
 - aarch64-elf-objcopy - to convert ELF executable into IMG format
 - aarch64-elf-objdump - utility to disassemble executables (for debugging)
 - aarch64-elf-readelf - an useful utility to dump sections and segments in executables (for debugging)

If you have all of the above six executables and you can also run them without error messages, congrats!
You have all the tools needed, you can start to work with my tutorials!
