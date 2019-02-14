/*
 * Copyright (C) 2019 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"
#include "sd.h"

// get the end of bss segment from linker
extern unsigned char _end;

// do not use the first sector (lba 0), could render your card unbootable
// choose a sector which is unused by your partitions
#define COUNTER_SECTOR 1

void main()
{
    // use the last 4 bytes on the second sector as a boot counter
    unsigned int *counter = (unsigned int*)(&_end + 508);
    // set up serial console
    uart_init();

    // initialize EMMC and detect SD card type
    if(sd_init()==SD_OK) {
        // read the second sector after our bss segment
        if(sd_readblock(COUNTER_SECTOR,&_end,1)) {
            // increase boot counter
            (*counter)++;
            // save the sector
            if(sd_writeblock(&_end,COUNTER_SECTOR,1)) {
                uart_puts("Boot counter ");
                uart_hex(*counter);
                uart_puts(" written to SD card.\n");
            }
        }
    }

    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
}
