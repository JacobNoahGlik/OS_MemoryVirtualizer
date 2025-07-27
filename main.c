#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "ram.h"
#include "pagetable.h"
#include "string.h"

void read_user_ip(unsigned char* RAM, bool p_pagetable, bool p_pagetable_b, bool ram_hex_dump);

// typedef void (*instr_handler_fn)(short, short, unsigned char, unsigned char*);
// 
// struct instruction_entry {
//     const char* name;
//     instr_handler_fn handler;
// };
// // Only uniform handlers go here
// struct instruction_entry instructions[] = {
//     { "map",   map },
//     { "store", store },
//     { "kill",  kill_process },
// };
// const int instruction_count = sizeof(instructions) / sizeof(instructions[0]);

void asign_bools(bool* pt, bool* pt_b, bool* ram, int argc, char* argv[]);


#include "disk.h"
int main(int argc, char* argv[]) {
    
    // assignes bool values based on argv (ex: ./main -debug_pagetable_b -help)
    bool p_pagetable = false, p_pagetable_b = false, ram_hex_dump = false;
    asign_bools(&p_pagetable, &p_pagetable_b, &ram_hex_dump, argc, argv);

    // setup
    unsigned char* RAM = new_ram(64);

    // read user input
    read_user_ip(RAM, p_pagetable, p_pagetable_b, ram_hex_dump);


    // cleanup
    delete_ram(RAM);
    return 0;
}


void asign_bools(bool* pt, bool* pt_b, bool* ram, int argc, char* argv[]) {
    char* flag_pt[4] = {"-debug_pagetable", "-debugPT", "-debug_pagetable_b", "-debugPT_b"};
    char* flag_ram[2] = {"-debug_ram", "-debugRAM"};
    for (short i = 1; i < argc; ++i) {
        if (string_compare(flag_pt[0], argv[i]) || string_compare(flag_pt[1], argv[i]))
            *pt = true;
        else if (string_compare(flag_pt[2], argv[i]) || string_compare(flag_pt[3], argv[i]))
            *pt_b = true;
        else if (string_compare(flag_ram[0], argv[i]) || string_compare(flag_ram[1], argv[i]))
            *ram = true;
        else if (string_compare("-help", argv[i]) || string_compare("-h", argv[i])) 
        {
            printf("RECCOGNIZED FLAGS...\n\tTo print out the pagetable in decimal every instruction: use flags ");
            printf("`%s` or `%s`\n\tTo print out the pagetable in binary every instruction:  use flags `%s` or ", flag_pt[0], flag_pt[1], flag_pt[2]);
            printf("`%s`\n\tTo hex-dump the ram every instruction:                   use flags `%s` or `%s`\n\n", flag_pt[3], flag_ram[0], flag_ram[1]);
            exit(0);
        }
        else
            printf("Unknown flag '%s', write -help for a list of flags\n", argv[i]);
    }
}


void read_user_ip(unsigned char* RAM, bool p_pagetable, bool p_pagetable_b, bool ram_hex_dump){

    int input_processID;
    char input_instructionType[5];
    int input_virtualAddress;
    int input_value;

    while (true)
    {

        printf("\nInstruction? ");
        scanf("%d,%5[^,],%d,%d", &input_processID, input_instructionType, &input_virtualAddress, &input_value);

        if (string_compare(input_instructionType, "map"))
        {
            map(input_processID, input_virtualAddress, input_value, RAM);
            // printf("Finished the Mapping instruction\n");
        }
        else if (string_compare(input_instructionType, "load"))
        {
            unsigned char eax;
            load(input_processID, input_virtualAddress, RAM, &eax);
            // printf("Finished the Loading instructions\n");
        }
        else if (string_compare(input_instructionType, "store"))
        {
            store(input_processID, input_virtualAddress, input_value, RAM);
            // printf("Finished Storing instructions\n");
        }
        else if (string_compare(input_instructionType, "exit")) {
            break;
        }
        else if (string_compare(input_instructionType, "kill")) {
            kill_process(input_processID, input_virtualAddress, input_value, RAM);
        }
        else
        {
            printf("#Error: Invalid instruction type...check your spelling\n");
        }

        if (p_pagetable)
            print_pagetable(RAM);
        if (p_pagetable_b)
            print_pagetable_b(RAM);
        if (ram_hex_dump)
            RAM_hex_dump(RAM, 64);
    }
}