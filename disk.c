#include "disk.h"
#include "pagetable.h"
#include <unistd.h>
#define __DISK_SIZE__ 1947 // 32 char + newline char per line, 63-4 lines per disk
void read_disk_content(char* arr);
void write_disk_content(char* arr);


unsigned char dec_to_hex(unsigned char dec, unsigned char unit) {
    if (unit == 0)
        dec = dec % 16;
    else
        dec /= 16;
    
    if (dec < 10) 
        return dec + 48;
    
    return dec + 55;
    
}
int hex_to_dec(unsigned char hex) {
    if (hex < 58 && 47 < hex)
        return hex - 48;
    else if (64 < hex && hex < 71)
        return hex - 55;
    else if (96 < hex && hex < 103)
        return hex - 87;
    return -1;
}

void write_to_disk(unsigned char* page, unsigned short pagenumber, unsigned short disk_number) {
    char disk[__DISK_SIZE__];
    read_disk_content(disk);
    page = &page[pagenumber * 16];
    int disk_pointer = (disk_number - 4) * 33;
    for (short bytenum = 0; bytenum < 16; ++bytenum) {
        // fputc(dec_to_hex(page[bytenum], 1), disk);
        // fputc(dec_to_hex(page[bytenum], 0), disk);
        disk[disk_pointer++] = dec_to_hex(page[bytenum], 1);
        disk[disk_pointer++] = dec_to_hex(page[bytenum], 0);
    }
    write_disk_content(disk);
}

void read_from_disk(unsigned char* page, unsigned short disk_number, unsigned short pagenumber) {
    char disk[__DISK_SIZE__];
    read_disk_content(disk);
    int disk_pointer = (disk_number - 4) * 33;

    page = &page[pagenumber * 16];

    char hex0, hex1;
    for (short bytenum = 0; bytenum < 16; ++bytenum) {
        // fscanf(disk, "%c", &hex0);
        // fscanf(disk, "%c", &hex1);
        hex0 = disk[disk_pointer++];
        hex1 = disk[disk_pointer++];
        page[bytenum] = hex_to_dec(hex0) * 16 + hex_to_dec(hex1);
    }
}

void write_to_page(unsigned char* RAM, unsigned char* tempRAM, unsigned short pagenumber) {
    unsigned char* page = &RAM[pagenumber * 16];
    for (short bytenum = 0; bytenum < 16; ++bytenum) {
        page[bytenum] = tempRAM[bytenum];
    }
}

void init_disk() {
    FILE* disk = fopen("disk", "w");

    for (short i = 4; i < 63; ++i) {
        fprintf(disk, "00000000000000000000000000000000\n");
    }

    fclose(disk);
}

void delete_disk() {
    remove("disk");
}

void read_disk_content(char* arr) {
    FILE* rfile = fopen("disk", "r");
    for (short line = 4; line < 63; ++line) {
        for (short letter = 0; letter < 33; ++letter) {
            fscanf(rfile, "%c", arr);
            arr++;
        }
    }
    fclose(rfile);
}

void write_disk_content(char* arr) {
    FILE* wfile = fopen("disk", "w");
    for (short line = 4; line < 63; ++line) {
        for (short letter = 0; letter < 33; ++letter) {
            fputc(*arr, wfile);
            arr++;
        }
    }
    fclose(wfile);
}