/*
 * Copyright (C) 2018 bzt (bztsrc@github)
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

#include "sd.h"
#include "uart.h"

// get the end of bss segment from linker
extern unsigned char _end;
unsigned int partitionlba;

// the BIOS Parameter Block (in Volume Boot Record)
typedef struct {
    char            jmp[3];
    char            oem[8];
    unsigned short  bps;
    unsigned char   spc;
    unsigned short  rsc;
    unsigned char   nf;
    unsigned short  nr;
    unsigned short  ts16;
    unsigned char   media;
    unsigned short  spf16;
    unsigned short  spt;
    unsigned short  nh;
    unsigned int    hs;
    unsigned int    ts32;
    unsigned int    spf32;
    unsigned int    flg;
    unsigned int    rc;
    char            vol[6];
    char            fst[8];
    char            dmy[20];
    char            fst2[8];
} __attribute__((packed)) bpb_t;

// directory entry structure
typedef struct {
    char            name[8];
    char            ext[3];
    char            attr[9];
    unsigned short  ch;
    unsigned int    attr2;
    unsigned short  cl;
    unsigned int    size;
} __attribute__((packed)) fatdir_t;

/**
 * Get the starting LBA address of the first partition
 * so that we know where our FAT file system starts, and
 * read that volume's BIOS Parameter Block
 */
int fat_getpartition()
{
    unsigned char *mbr=&_end;
    bpb_t *bpb=(bpb_t*)&_end;
    // read the partitioning table
    if(sd_readblock(0,&_end,1)) {
        // check magic
        if(mbr[510]!=0x55 || mbr[511]!=0xAA) {
            uart_puts("ERROR: Bad magic in MBR\n");
            return 0;
        }
        // check partition type
        if(mbr[0x1C2]!=0xE/*FAT16 LBA*/ && mbr[0x1C2]!=0xC/*FAT32 LBA*/) {
            uart_puts("ERROR: Wrong partition type\n");
            return 0;
        }
        // gcc generates bad code for this...
        //partitionlba=*((unsigned int*)((unsigned long)&_end+0x1C6));
        partitionlba=mbr[0x1C6] + (mbr[0x1C7]<<8) + (mbr[0x1C8]<<16) + (mbr[0x1C9]<<24);
        // read the boot record
        if(!sd_readblock(partitionlba,&_end,1)) {
            uart_puts("ERROR: Unable to read boot record\n");
            return 0;
        }
        // check file system type. We don't use cluster numbers for that, but magic bytes
        if( !(bpb->fst[0]=='F' && bpb->fst[1]=='A' && bpb->fst[2]=='T') &&
            !(bpb->fst2[0]=='F' && bpb->fst2[1]=='A' && bpb->fst2[2]=='T')) {
            uart_puts("ERROR: Unknown file system type\n");
            return 0;
        }
        return 1;
    }
    return 0;
}

/**
 * Find a file in root directory entries
 */
unsigned int fat_getcluster(char *fn)
{
    unsigned char *vbr=&_end;
    bpb_t *bpb=(bpb_t*)&_end;
    fatdir_t *dir=(fatdir_t*)(&_end+512);
    unsigned int root_sec, s;
    // find the root directory's LBA
    root_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
    //WARNING gcc generates bad code for bpb->nr, causing unaligned exception
    s=vbr[17] + (vbr[18]<<8);
    s<<=5;
    // now s=bpb->nr*sizeof(fatdir_t));
    if(bpb->spf16==0) {
        // adjust for FAT32
        root_sec+=(bpb->rc-2)*bpb->spc;
    }
    // add partition LBA
    root_sec+=partitionlba;
    // load the root directory
    if(sd_readblock(root_sec,(unsigned char*)dir,s/512+1)) {
        // iterate on each entry and check if it's the one we're looking for
        for(;dir->name[0]!=0;dir++) {
            // is it a valid entry?
            if(dir->name[0]==0xE5 || dir->attr[0]==0xF) continue;
            // filename match?
            if(!__builtin_memcmp(dir->name,fn,11)) {
                uart_puts("FAT File ");
                uart_puts(fn);
                uart_puts(" starts at cluster: ");
                uart_hex(((unsigned int)dir->ch)<<16|dir->cl);
                uart_puts("\n");
                // if so, return starting cluster
                return ((unsigned int)dir->ch)<<16|dir->cl;
            }
        }
        uart_puts("ERROR: file not found\n");
    } else {
        uart_puts("ERROR: Unable to load root directory\n");
    }
    return 0;
}

/**
 * Read a file into memory
 */
char *fat_readfile(unsigned int cluster)
{
    // BIOS Parameter Block
    unsigned char *vbr=&_end;
    bpb_t *bpb=(bpb_t*)&_end;
    // File allocation tables. We choose between FAT16 and FAT32 dynamically
    unsigned int *fat32=(unsigned int*)(&_end+bpb->rsc*512);
    unsigned short *fat16=(unsigned short*)fat32;
    // Data pointers
    unsigned int data_sec, s;
    unsigned char *data, *ptr;
    // find the LBA of the first data sector
    data_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
    //WARNING gcc generates bad code for bpb->nr, causing unaligned exception
    s=vbr[17] + (vbr[18]<<8);
    s<<=5;
    if(bpb->spf16>0) {
        // adjust for FAT16
        data_sec+=(s+511)>>9;
    }
    // add partition LBA
    data_sec+=partitionlba;
    // dump important properties
    uart_puts("FAT Bytes per Sector: ");
    //uart_hex(bpb->bps);
    uart_hex(vbr[11] + (vbr[12]<<8));
    uart_puts("\nFAT Sectors per Cluster: ");
    uart_hex(bpb->spc);
    uart_puts("\nFAT Number of FAT: ");
    uart_hex(bpb->nf);
    uart_puts("\nFAT Sectors per FAT: ");
    uart_hex((bpb->spf16?bpb->spf16:bpb->spf32));
    uart_puts("\nFAT Reserved Sectors Count: ");
    uart_hex(bpb->rsc);
    uart_puts("\nFAT First data sector: ");
    uart_hex(data_sec);
    uart_puts("\n");
    // load FAT table
    s=sd_readblock(partitionlba+1,(unsigned char*)&_end+512,(bpb->spf16?bpb->spf16:bpb->spf32)+bpb->rsc);
    // end of FAT in memory
    data=ptr=&_end+512+s;
    // iterate on cluster chain
    while(cluster>1 && cluster<0xFFF8) {
        // load all sectors in a cluster
        sd_readblock((cluster-2)*bpb->spc+data_sec,ptr,bpb->spc);
        // move pointer, sector per cluster * bytes per sector
        ptr+=bpb->spc*bpb->bps;
        // get the next cluster in chain
        cluster=bpb->spf16>0?fat16[cluster]:fat32[cluster];
    }
    return (char*)data;
}

