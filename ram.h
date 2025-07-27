#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

unsigned char* new_ram(int num_bytes);
void delete_ram(unsigned char* ram);

unsigned char* RAM_get_page(unsigned char* RAM, unsigned short index);


void RAM_hex_dump(unsigned char* RAM, unsigned short ram_max);
void print_pagetable(unsigned char* RAM);
void print_pagetable_b(unsigned char* RAM);

void map(
    short process_id, 
    short byte_number,
    bool read_and_write,
    unsigned char *RAM
);

void load(
    short process_id,
    short byte_number,
    unsigned char *RAM,
    unsigned char *eax
);

void store(
    short process_id,
    short byte_number,
    unsigned char value,
    unsigned char *RAM
);