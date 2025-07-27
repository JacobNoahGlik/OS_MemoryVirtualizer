#include <unistd.h> // used for checking the existance of files
#include "pagetable.h"
#include "disk.h"
#include "tracker.h"
#include "ram.h"

// how is the pt_row_placeholder orgenised/obfuscated?
/* 
typedef struct pt_r {
    4bit process_id;  // byte0   (values 0-15)                       start: 0
    7bit[4] phys_t;   // byte0-3 (  values: array of 
                                    6bit physical mem location {4-9, 11-16, 18-23, 25-30} 
                                    1bit isWriteable bool      {10, 17, 24, 31}
                                    NOTE: `phys_mem == 63 ? unassigned : assigned`
                                    NOTE: `phys_mem > 3 ? disk : ram`
                                 ) {index of arr is virtual page ID} start: 4, 11, 18, 25
} PageTable_Row; */



// START PRIVATE FUNCTION DECLARATION !!!!!!!!!!

// setters
void pt_set_procID(short new_proc_id, PageTable_Row* row);
void pt_set_readOnly(bool rO, PageTable_Row* row, unsigned short index);
void pt_set_baseReg(short new_base_reg, PageTable_Row* row, unsigned short index);

void pt_SETALL_zero(PageTable_Row* row);

unsigned short pt_swap_page(unsigned char* RAM, unsigned short pID_DISK, unsigned short pID_RAM);

// helpers
PageTable_Row* get_PageTable(unsigned char* RAM) { return (PageTable_Row*)RAM; }
bool row_is_unassigned(PageTable_Row* row);
int get_index_of_next_empty(PageTable_Row* PageTable, int max_rows);
void print_byte_b(unsigned char byte);
void print_byte_bFormat(unsigned char byte, short index);
void print_byte_h(unsigned char byte);
bool is_offset(unsigned char byte, short index);
bool is_offset_PTR(PageTable_Row* byte, short index);
void set_bit(unsigned char* byte, short index);
void unset_bit(unsigned char* byte, short index);
void slowset_bit(unsigned char* byte, short index, bool value);
void set_bit_PTR(PageTable_Row* row, short index);
void unset_bit_PTR(PageTable_Row* row, short index);
bool page_busy(PageTable_Row* PT, unsigned short pagenumber);

unsigned char least_used_page();
// END PRIVATE FUNCTION DECLARATION !!!!!!!!!!



// START PRIVATE IMPLEMENTATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

short pt_get_procID(PageTable_Row* row) {
    // since procID is the first six bits...
    // bin:1111 0000 = dec:15
    return (*((unsigned char*)row) & 15) - 1;
}
bool pt_is_empty(PageTable_Row* row) {
    return pt_get_procID(row) == -1;
}
bool pt_is_readOnly(PageTable_Row* row, unsigned short index) {
    return !is_offset_PTR(row, index * 7 + 10);
}
bool pt_is_read_and_write(PageTable_Row* row, unsigned short index) { return !pt_is_readOnly(row, index); }
bool pt_is_evicted(PageTable_Row* row, unsigned short index) {
    return (pt_get_baseReg(row, index) > 3); // if grator then 3, must be in disk
}
short pt_get_baseReg(PageTable_Row* row, unsigned short index) {
    short sum = 0;
    short startbyte = index * 7 + 4;
    for (short i = startbyte + 5; i >= startbyte; --i) {
        sum *= 2;
        sum += is_offset_PTR(row, i);
    }
    return sum;
}

void pt_set_procID(short new_proc_id, PageTable_Row* row) {
    new_proc_id++;
    if (new_proc_id > 15 || 1 > new_proc_id) {
        const char* func_name = "function pt_set_procID(short new_proc_id, PageTable_Row* row)";
        printf("\nERROR: %s got @param::new_proc_id %d when expected 0 <= npID <= 14\n", func_name, new_proc_id);
        exit(1);
    }

    unsigned char* p = (void*)row;
    *p = *p & 240; // dec:240 = bin:0000 1111 (which will zero the first four bits)
    *p = *p | new_proc_id; // since the proc id is the first 4 bits, or-ing it will only change those bits to 1's
}
void pt_set_readOnly(bool rO, PageTable_Row* row, unsigned short index) {
    if (rO)
        unset_bit_PTR(row, index * 7 + 10);
    else
        set_bit_PTR(row, index * 7 + 10);
}
// sets all physical_registers to __REG_UNASSIGNED__ (i.e. int:63)
void pt_set_BR_unassigned(PageTable_Row* row) {
    for (short index = 0; index < 4; ++index) {
        short startbyte = index * 7 + 4;
        short baseRegSize = 6; // each reg is 6 bytes
        for (short i = startbyte; i < startbyte + baseRegSize; ++i) { set_bit_PTR(row, i); } // set all true
    } // value 63 (__REG_UNASSIGNED__) is reserved to mean unassigned
}
void pt_set_baseReg(short new_base_reg, PageTable_Row* row, unsigned short index) {
    if (new_base_reg > 62 || 0 > new_base_reg) {
        const char* func_name = "function pt_set_baseReg(short new_base_reg, PageTable_Row* row)";
        printf("\nERROR: %s got @param::new_base_reg %d when expected 0 <= npID <= 62\n", func_name, new_base_reg);
        exit(1);
    } // note value 63 is reserved to mean unassigned

    short startbyte = index * 7 + 4;
    short baseRegSize = 6; // each reg is 6 bytes
    for (short i = startbyte; i < startbyte + baseRegSize; ++i) {
        if (new_base_reg % 2 != 0)
            set_bit_PTR(row, i);
        else
            unset_bit_PTR(row, i);
        new_base_reg /= 2;
    }
}

void pt_SETALL_zero(PageTable_Row* row) {
    row->data = 0; // set the unsigned int to zero (represented by 32 bites at value zero in memory)
}

bool row_is_unassigned(PageTable_Row* row) {
    for (short i = 0; i < 4; ++i) { // check each register
        if (pt_get_baseReg(row, i) != __REG_UNASSIGNED__) // if any are assigned
            return false; // return false
    }
    return true; // to get here, all registers were unassigned
}

int get_index_of_next_empty(PageTable_Row* PageTable, int max_rows) {
    int pt_index = 0;
    while (pt_index < max_rows) {
        if (row_is_unassigned(&PageTable[pt_index]))
            return pt_index;
        pt_index++;
    }
    return -1; // pagetable is full
}

void print_byte_b(unsigned char byte) {
    for (short i = 0; i < 8; ++i) {
        printf("%d", byte % 2);
        byte /= 2;
        if (i == 3)
            printf(" ");
    }
}

void print_byte_bFormat(unsigned char byte, short index) {
    index = index * 8;
    for (short i = 0; i < 8; ++i) {
        printf("%d", byte % 2);
        byte /= 2;
        if (index == 3 || index == 10 || index == 17 || index == 24)
            printf(" ");
        if (index == 9 || index == 16 || index == 23 || index == 30)
            printf("-");
        index++;
    }
}
void print_byte_h(unsigned char byte) {
    printf("%c%c", dec_to_hex(byte, 1), dec_to_hex(byte, 2));
}
bool is_offset(unsigned char byte, short index) {
    unsigned char all_zero[8] = {1, 2, 4, 8, 16, 32, 64, 128}; // {1000 0000, 0100 0000, 0010 0000, ...}
    return (byte & all_zero[index]) != 0;
}
bool is_offset_PTR(PageTable_Row* byte, short index) {
    unsigned char* tmp = (void*)byte;
    return is_offset(tmp[index / 8], index % 8);
}
void set_bit(unsigned char* byte, short index) {
    unsigned char y[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    *byte = *byte | y[index];
}
void unset_bit(unsigned char* byte, short index) {
    unsigned char n[8] = {254, 253, 251, 247, 239, 223, 191, 127};
    *byte = *byte & n[index];
}
void slowset_bit(unsigned char* byte, short index, bool value) {
    unsigned char y[8] = {1, 2, 4, 8, 16, 32, 64, 128}; // {1000 0000, 0100 0000, 0010 0000, 0001 0000...}
    unsigned char n[8] = {254, 253, 251, 247, 239, 223, 191, 127}; // {0111 1111, 1011 1111, 1101 1111, ...}
    if (value) // if else is very expencive (about 6 - 9 clock-beets)
        *byte = *byte | y[index];
    else   
        *byte = *byte & n[index];
}
void set_bit_PTR(PageTable_Row* row, short index) {
    unsigned char* ptr = (void*)row;
    set_bit(&ptr[index / 8], index % 8);
}
void unset_bit_PTR(PageTable_Row* row, short index) {
    unsigned char* ptr = (void*)row;
    unset_bit(&ptr[index / 8], index % 8);
}



// will make sure the page id is on ram
void load_page(PageTable_Row* row, unsigned short pageID) {
    // TODO IMPLEMENTATION
    printf("\n\n\tINCOMPLETE CODE THIS HAS TO BE EDITED\n\n\n");
}
void set_value(unsigned char* RAM, PageTable_Row* row, unsigned short byteNumber, unsigned char value) {
    unsigned short physicalID = pt_get_baseReg(row, byteNumber / 16);
    RAM[physicalID * 16 + byteNumber % 16] = value;
}
unsigned char get_value(unsigned char* RAM, PageTable_Row* row, unsigned short byteNumber) {
    unsigned short physicalID = pt_get_baseReg(row, byteNumber / 16);
    return RAM[physicalID * 16 + byteNumber % 16];
}

unsigned short pt_swap_page(unsigned char* RAM, unsigned short pID_DISK, unsigned short pID_RAM) {
    unsigned char tempRAM[16];               // int temp
    read_from_disk(tempRAM, pID_DISK, 0);    // temp = a
    write_to_disk(RAM, pID_RAM, pID_DISK);   // a = b
    write_to_page(RAM, tempRAM, pID_RAM);    // b = temp

    PageTable_Row* row = (void*)RAM; // swap values of physical address saved in pagetable
    bool dswap = true, rswap = true;
    while (dswap || rswap) {
        for (short i = 0; i < 4; ++i) {
            short br = pt_get_baseReg(row, i);
            if (br == pID_DISK) {
                pt_set_baseReg(pID_RAM, row, i);
                dswap = false;
            } else if (br == pID_RAM) {
                pt_set_baseReg(pID_DISK, row, i);
                rswap = false;
            }

            if (!dswap && !rswap)
                break;
        }

        row++;
    } // swap complete

    printf("Swapped physical frame %d (on RAM) to swap-slot %d (represented as %d in pagetable)\n", pID_RAM, pID_DISK - 4, pID_DISK);
    printf("Swapped swap-slot %d (represented as %d in pagetable) to physical frame %d (on RAM)\n", pID_DISK - 4, pID_DISK, pID_RAM);
    return pID_RAM;
}

// END PRIVATE IMPLEMENTATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//

//

//

// START PUBLIC IMPLEMENTATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!


void init_page_table(unsigned char* RAM) {
    PageTable_Row* page_table = get_PageTable(RAM);
    int pt_size = __RAM_SIZE__ / 4;
    // we know a row of a page_table has not been used if the limit is zero and evicted is false
    for (int i = 0; i < pt_size; ++i) {
        PageTable_Row* current_row = &page_table[i];
        pt_SETALL_zero(current_row);
        pt_set_BR_unassigned(current_row); // set row as unasigned
    }
    // start the tracker on page1 because page0 is used by pagetable
    tracker_init(1, __TRACKER_FILENAME__);
    init_disk();
}

void delete_page_table(unsigned char* RAM) {
    delete_disk();
    delete_tracker(__TRACKER_FILENAME__);
}

void pagetable_print_row(unsigned char* RAM, unsigned short row_index) {
    if (row_index > __RAM_SIZE__ / 4) // out of bounds
        return;

    PageTable_Row* page_table = get_PageTable(RAM);
    PageTable_Row* row = &page_table[row_index];
    printf("ProcID:%d, phys:{", pt_get_procID(row));
    for (short i = 0; i < 3; ++i) {
        short br_val = pt_get_baseReg(row, i);
        const char* rw = (pt_is_readOnly(row, i) ? "r), " : "rw),");
        //printf(br_val != __REG_UNASSIGNED__ ? "%d", br_val : "unassigned");
        if (br_val == __REG_UNASSIGNED__)
            printf("%d -> -1 (-1), ", i);
        else if (br_val < 10)
            printf("%d -> %d (%s  ", i, br_val, rw);
        else
            printf("%d -> %d (%s ", i, br_val, rw);
    }
    short br_val = pt_get_baseReg(row, 3);
    const char* rw = (pt_is_readOnly(row, 3) ? "r)}\n" : "rw)}\n");
    if (br_val == __REG_UNASSIGNED__)
        printf("3 -> -1 (-1)}\n");
    else if (br_val < 10)
        printf("3 -> %d (%s  ", br_val, rw);
    else
        printf("3 -> %d (%s ", br_val, rw);
    
}

void pagetable_print(unsigned char* RAM) {
    printf("\t.PAGE_TABLE___________RAM_PAGE_0.\n");
    printf("\t|PID phMem0 phMem1 phMem2 phMem3|\n\t|");
    for(int i = 0; i < __RAM_SIZE__; i += 4) {
        PageTable_Row* row = (void*)&RAM[i];
        int ProcID = pt_get_procID(row);
        int physical[4], rw[4];
        for (short j = 0; j < 4; ++j) {
            physical[j] = pt_get_baseReg(row, j);
            rw[j] = !pt_is_readOnly(row, j);
        }

        if (ProcID == -1)
            printf("NA  ");
        else if (ProcID < 10)
            printf("%d   ", ProcID);
        else
            printf("%d  ", ProcID);
        
        for (short j = 0; j < 4; ++j) {
            if (physical[j] == __REG_UNASSIGNED__)
                printf("NA ");
            else
                printf("%d ", physical[j]);
            
            if (j != 3)
                printf(rw[j] ? "r/w " : "r/  ");
            else
                printf(rw[j] ? "r/w" : "r/ ");

            if (physical[j] < 10)
                printf(" ");
        }
        printf("|\n\t|");
    }
    printf("_______________________________|\n");
}
void pagetable_print_binary_formatflag(unsigned char* RAM) {
    short index = 0;
    printf("\t.BINARY_PAGE_TABLE_____________RAM_PAGE_0.\n");
    printf("\t|PrID physMem0 physMem1 physMem2 physMem3|\n\t|");
    while (index < __RAM_SIZE__) {
        print_byte_bFormat(RAM[index], index % 4);
        if (index % 4 == 3)
            printf("|\n\t|"); // printf(" %p\n", (void*)&RAM[index - 3]); // for debug print address
        index++;
    }
    printf("________________________________________|\n");
}

const PageTable_Row* get_pt_row(unsigned char* RAM, const int index) {
    if (index < 0 || index > __RAM_SIZE__ * 4) 
        return NULL;// if index out of bounds return NULL

    PageTable_Row* temp_row = &get_PageTable(RAM)[index];
    // if (pt_get_limitReg(temp_row))
    //     return NULL; // if pagetable[index] = empty (un-initialized) -> return null
    
    return temp_row;
}


// MAIN FUNCTIONS



void map_helper(short process_id, short byte_number, bool read_and_write, unsigned char *RAM) {

    PageTable_Row* row = (void*)RAM;
    short vPage = byte_number / 16;
    for (short i = 0; i < 4; ++i) {

        if (pt_is_empty(row)) { // if process id isn't in the table yet
            pt_set_procID(process_id, row); // add it to the table

            // startbyte is current row - start of ram
            int startbyte = (void*)row - (void*)RAM;
            // end byte is startbyte plus sizeof(row) = 4
            // int endbyte = startbyte + 4;
            printf("WARNING: all pagetables reside in physical frame 0, but only take 4 bytes each.\n");
            printf("Added page table for PID %d into physical frame 0 (bytes %d, %d, %d, and %d)\n", process_id, startbyte, startbyte+1, startbyte+2, startbyte+3);

            break; // exit with the right row pointer
        }

        if (pt_get_procID(row) == process_id) // if reached right row pointer
            break; // exit with the right row pointer

        row++; // increment the row pointer
    }

    if (pt_get_baseReg(row, vPage) == __REG_UNASSIGNED__) 
    {
        short frame_number = tracker_getAvailablePage(__TRACKER_FILENAME__);
        printf("Mapped virtual address %d (page %d) into physical frame %d ", byte_number, vPage, frame_number);
        printf(frame_number < 4 ? "(on RAM)\n" : "(on DISK)\n");
        // set protection bit
        pt_set_readOnly(!read_and_write, row, vPage);

        // set physical page location
        pt_set_baseReg(
            // get next available page location (even if on disk)
            frame_number, 
            row,
            vPage
        );
    }
    else if (pt_is_read_and_write(row, vPage) != read_and_write) 
    {
        // if protection bit not in agreement: override it
        pt_set_readOnly(!read_and_write, row, vPage);
        printf("Updating permissions for virtual page %d (frame %d)\n", vPage, pt_get_baseReg(row, vPage));
    }
    else
    {
        // throw error if already mapped and if protection bit is in agreement
        printf("Error: virtual page %d is already mapped with rw_bit=%d\n", vPage, (int)pt_is_read_and_write(row, vPage));
    }

}


unsigned char load_helper(short process_id, short byte_number, unsigned char *RAM ) {
    PageTable_Row* row = (void*)RAM;
    for (short i = 0; i < 4; ++i) {

        if (pt_is_empty(row)) { // if process id isn't in the table yet
            printf("ERROR: PROCESS %d IS NOT IN PAGETABLE YET\n", process_id);
            return 0; // throw error
        }

        if (pt_get_procID(row) == process_id) // if reached right row pointer
            break; // exit with the right row pointer

        row++; // increment the row pointer
    }

    short vPage = byte_number / 16;
    short phyLocation = pt_get_baseReg(row, vPage);

    if (phyLocation == __REG_UNASSIGNED__) { // not mapped yet
        printf("ERROR: PAGE %d OF PROCESS %d HAS NOT BEEN MAPPED YET\n", vPage, process_id);
        return 0;
    }

    if (phyLocation > 3) // if on disk
        phyLocation = pt_swap_page(RAM, phyLocation, tracker_least_used()); // pull from disk to RAM
        // tracker_least_used() -> returns the page number that has been most idle

    tracker_update_lu(phyLocation); // notify tracker that we are using this page

    short true_bytenumber = phyLocation * 16 + byte_number % 16;
    printf("The value %d is virtual address %d (physical address %d)\n", RAM[true_bytenumber], byte_number, true_bytenumber);
    return RAM[true_bytenumber];
}

void store_helper(short process_id, short byte_number, unsigned char value, unsigned char *RAM) {
    PageTable_Row* row = (void*)RAM;
    for (short i = 0; i < 4; ++i) {

        if (pt_is_empty(row)) { // if process id isn't in the table yet
            printf("ERROR: PROCESS %d IS NOT IN PAGETABLE YET\n", process_id);
            return; // throw error
        }

        if (pt_get_procID(row) == process_id) // if reached right row pointer
            break; // exit with the right row pointer

        row++; // increment the row pointer
    }

    short vPage = byte_number / 16;
    if (pt_is_readOnly(row, vPage)) {
        printf("Error: writes are not allowed to this page\n");
        return;
    }
    short phyLocation = pt_get_baseReg(row, vPage);

    if (phyLocation == __REG_UNASSIGNED__) {
        printf("ERROR: PAGE %d OF PROCESS %d HAS NOT BEEN MAPPED YET\n", vPage, process_id);
        return;
    }

    if (phyLocation > 3) // move from disk to RAM
        phyLocation = pt_swap_page(RAM, phyLocation, tracker_least_used(__TRACKER_FILENAME__));
    
    tracker_update_lu(phyLocation); // notify tracker that we are using this page

    short true_bytenumber = phyLocation * 16 + byte_number % 16;
    RAM[true_bytenumber] = value;
    printf("Stored value %d at virtual address %d (physical address %d)\n", value, byte_number, true_bytenumber);
}


void kill_process(short process_id, short byte_number, unsigned char value, unsigned char *RAM) {
    if (process_id < 0 || process_id > 15) {
        printf("#Error: PID must be between 0 and 15\n");
        return;
    }

    for (int i = 0; i < 4; i++) {
        PageTable_Row* row = (PageTable_Row*)&RAM[i * 4];
        short stored_pid = pt_get_procID(row);

        // check if blank
        if (stored_pid == -1) {
            continue;
        }

        if (stored_pid == process_id) {
            *(int32_t*)row = __UNUSED_PAGETABLE_ENTRY__;
            printf("Process %d killed and its page table entry cleared.\n", process_id);
            return;
        }
    }
    printf("ERROR: Process %d not found in page table.\n", process_id);
}