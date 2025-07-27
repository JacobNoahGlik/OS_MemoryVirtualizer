#include "ram.h"
#include "pagetable.h"
#include "disk.h"

// private
unsigned char byte_to_hex(unsigned char dec, unsigned char unit);
void rm_disks();



unsigned char* new_ram(int num_bytes) {
    unsigned char* temp = (unsigned char*)malloc(sizeof(char) * num_bytes + 1);
    temp[num_bytes] = '\0'; // force null-termination
    init_page_table(temp);
    return temp;
}


void delete_ram(unsigned char* ram) {
    delete_page_table(ram);
    delete_disk();
    free(ram);
}

unsigned char* RAM_get_page(unsigned char* RAM, unsigned short index) {
    return &RAM[index*16];
}

void RAM_hex_dump(unsigned char* RAM, unsigned short ram_max) {
    printf("\t.RAM_HEX_DUMP_______________________.\n\t|");
    char byte;
    for (unsigned short i = 0; i < ram_max; ++i) {
        byte = RAM[i];
        printf("%c%c", byte_to_hex(byte, 1), byte_to_hex(byte, 0));
        if (i % 4 == 3 && i % 16 != 15)
            printf(" ");
        if (i % 16 == 15)
            printf("|\n\t|");
    }
    printf("___________________________________|\n");
}

void print_pagetable(unsigned char* RAM) {pagetable_print(RAM);}
void print_pagetable_b(unsigned char* RAM) {pagetable_print_binary_formatflag(RAM);}

unsigned char byte_to_hex(unsigned char byte, unsigned char unit) {
    if (unit == 0)
        byte = byte % 16;
    else
        byte /= 16;
    
    if (byte < 10) 
        return byte + 48;
    
    return byte + 55;
    
}

void rm_disks() {
    char disk_name[7];
    for (short disk_number = 4; disk_number < 63; ++disk_number) {
        sprintf(disk_name, "%d.disk", disk_number);
        remove(disk_name);
    }
}


void map(short process_id, short byte_number, bool read_and_write, unsigned char *RAM) {map_helper(process_id, byte_number, read_and_write, RAM);}

void load(short process_id, short byte_number, unsigned char *RAM, unsigned char* eax) {*eax = load_helper(process_id, byte_number, RAM);}

void store(short process_id, short byte_number, unsigned char value, unsigned char *RAM) {store_helper(process_id, byte_number, value, RAM);}