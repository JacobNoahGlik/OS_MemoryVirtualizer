#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void write_to_disk(unsigned char* page, unsigned short pagenumber, unsigned short disk_number);
void read_from_disk(unsigned char* page, unsigned short disk_number, unsigned short pagenumber);
void write_to_page(unsigned char* RAM, unsigned char* tempRAM, unsigned short pagenumber);

void init_disk();
void delete_disk();

unsigned char dec_to_hex(unsigned char dec, unsigned char unit);
int hex_to_dec(unsigned char hex);