#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define __REG_UNASSIGNED__ 63
#define __RAM_SIZE__ 16
#define __TRACKER_FILENAME__ "nextpage.tracker"
#define __UNUSED_PAGETABLE_ENTRY__ ((int32_t)0x7EFDFBF0)
// how is the pt_row_placeholder orgenised/obfuscated?
/* 
typedef struct pt_r {
    4bit process_id;  // byte0   (values 0-7)                        start: 0
    7bit[4] phys_t;   // byte0-3 (  values: array of 
                                    6bit physical mem location
                                    1bit isWriteable bool
                                 ) {index of arr is virtual page ID} start: 4, 11, 18, 25
} PageTable_Row; */


// row obfuscation...
// 16 bytes can fit up to 4 rows of process info
typedef struct pt_row_placeholder {
    unsigned int data; // compilererror
} PageTable_Row;

/// @brief Initialize new pagetable in RAM from bytes 0 to macro::__RAM_SIZE__ defined in pagetable.c
/// @param RAM pointer to the start of RAM
void init_page_table(unsigned char* RAM);
void delete_page_table(unsigned char* RAM);
// PageTable_Row* get_PageTable(unsigned char* RAM);
// const PageTable_Row* get_pt_row(unsigned char* RAM, const int pt_byte_size, const int index);


// getters
short pt_get_procID(PageTable_Row* row);
bool pt_is_empty(PageTable_Row* row);
bool pt_is_readOnly(PageTable_Row* row, unsigned short index);
bool pt_is_read_and_write(PageTable_Row* row, unsigned short index);
bool pt_is_evicted(PageTable_Row* row, unsigned short index);
short pt_get_baseReg(PageTable_Row* row, unsigned short index);


void pagetable_print_row(unsigned char* RAM, unsigned short row_index);
void pagetable_print_binary_formatflag(unsigned char* RAM);
void pagetable_print(unsigned char* RAM);

void kill_process(short process_id, short byte_number, unsigned char value, unsigned char *RAM);

void map_helper(
    short process_id, 
    short byte_number,
    bool read_and_write,
    unsigned char *RAM
);

unsigned char load_helper(
    short process_id,
    short byte_number,
    unsigned char *RAM
);

void store_helper(
    short process_id,
    short byte_number,
    unsigned char value,
    unsigned char *RAM
);