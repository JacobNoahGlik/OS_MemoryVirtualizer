NOTE: this is a converted-to-txt verson of the real read-me (README.md) which is better formated if opened in preview



Project3
Operating Systems Memory Manager



Make commands
  make
  make clean
Run program
  ./main
Run with debug flags
  ./main -help
will not run, but will display all known flags
  ./main -debug_pagetable
will run and will display pagetable before and after each command is given to the program
  ./main -debug_pagetable_b
will run and will display pagetable (in binary bits) before and after each command is given to the program
  ./main -debug_ram
will run and will display ram (in hex) before and after each command is given to the program
  ./main -debugPT -debugPT_b -debugRAM
will run with all debug tools
NOTE: these flags can all be chained as long as -h / -help is NOT given because those flags kill the program



Legal Input
  %d<$ProcessID>, %s<$instruction>, %d<$eax>, %d<$ebx>
      $ProcessID => value from 0 - 14 (project only requires 0-3 but we are able to support more)
      $instruction => values: "map", "store", "load", "exit"
          map -> reserves physical location and sets read/write protection bit ($ebx == 0 ? 0 : 1) for virtual byte in 4 ranges (0-15, 16-31, 32-47, 48-63)
          store -> saves $ebx to virtual address $eax as long as $eax is in a virtual page that has already been mapped
          load -> retrieves value saved in virtual address $eax as long as virtual page has already been mapped
          exit -> ends run. dealocates all memory alocated durring runtime, whipes DISK and tracker.
      $eax => value from 0 - 63 (virtual byte)
      $ebx => value from 0 - 255 (setter value unless irrelevant: i.e. functions load and exit)



How our PageTables work
Note: to see the back-end and how our pagetables work in real-time add flag -debug_pagetable_b to see in bits or -debug_pagetable to see in decimal
  The first page in RAM (frame0) (16bytes) is reserved for all four page tables
  Our pagetables are 4 bytes in size. The first 4 bits (bits 0-3) store the following atribute:
      Process ID
            Size: 4 bits
            A value from 0-15
            Value 0 means no process asigned
            Value 1-15 is one higher then the process id (supports process ids 0-14)
  The next 28 bits (bits 4-31) store an array of size 4 where each member is 7 bits. (4-10, 11-17, 18-24, 25-31) The members are:
      Physical Page Location for Virtual Page(index of array)
          Size: 6 bits
          A value from 0-63
          A value of 63 (0b 1111 11) means this virtual page has not yet been mapped to a physical frame
          A value of 0-3 (0b 0000 00 - 0b 1100 00) means this virtual page has been mapped to a physical frame in RAM
          NOTE: that a physical frame of 0 (frame0) is impossible because the PageTables reserve that slot
          A value of 4-62 (0b 0010 00 - 0b 0111 11) means this virtual page has been mapped to a physical frame in DISK (file:"disk")
      Protection / ReadWrite Bit for Virtual Page(index of array)
          Size: 1 bit
          Value of 0 (false) means this page cannot be written to, only read from
          Value of 1 (true) means this page can be both written to and read from



How our functions work (map, store, load, exit)
  map
      Takes int ProcessID(0-14), int byte_requested(0-63), bool protection_bit(0-1)
      Adds ProcessID to pagetable if pagetale does not yet have this ProcessID
      Requests next available page (1-62) from tracker (which keeps track of which page is available, and increments tracker when called)
      Sets respective physical page slot to next available page recieved and sets protection bit
  store
      Takes int ProcessID(0-14), int virtual_byte(0-63), char value(0-255)
      Checks to make sure ProcessID is already in pagetable (returns / error-s if false)
      Checks to make sure virtual_byte is in page already alocated to a frame (returns / error-s if false)
      Checks to make sure that frame is on RAM (1-3) (swaps if false)
      Sets respective byte to the value of char value(0-255) in RAM
  load
      Takes int ProcessID(0-14), int virtual_byte(0-63), char *address_of_register(unused in current setup)
      Checks to make sure ProcessID is already in pagetable (returns / error-s if false)
      Checks to make sure virtual_byte is in page already alocated to a frame (returns / error-s if false)
      Checks to make sure that frame is on RAM (1-3) (swaps if false)
      Sets register in address address_of_register to value recieved from RAM
  exit
      Stops process, and dealocates all allocated memory (RAM / DISK / TRACKER / etc)



How we handle swaping of frames
  Locate frame in DISK we need
      Given by calling process when calling function swap()
  Locate frame in RAM we will swap to
      Tracker tells us which frame has been idle the longest (that's the frame we chose)
      NOTE: this is better then round-robbin because if a process is constantly using a frame it will be subbed out to DISK in round-robin, but won't in our method
  Swap the two frames
      Save the frame in RAM to a temporary DISK row
      Move the wanted DISK row into the frame in RAM
      Move the temporary DISK row into wanted frame in RAM
  Update the Pagetables
      Search each array in the pagetable until you find DISK frame or RAM frame
      Swap them to the other (because those frames have been swapped)



How the DISK works
  One single file named "disk"
  Gets created once program is run, and removed when program exits
  Each row stands for frames 4-62 (disk_frams 0-58)
  Disk is in hex where 2 chars stand for one byte of hex (0x00 - 0xFF -> 0 - 255)
  Init sets each row to 32 0s (16 bytes of zero / NULL)



How the tracker works
  Tracking most idle frame in RAM
      Implemented as a stack in DISK
      init creates tracker file with value "1 2 3"
      Meaning 1 is most idle, while 2 is next-most dile, and 3 is least idle
      Each time a page is used it gets removed from the file, and gets added to the end
          Example: 2 is used -> remove: "1 3", add to end: "1 3 2", 1 is still most idle, but now 2 is least idle, while 3 has moved to second-most idle
      Each time a most-idle page is requested the first value is returned
  Tracking what the next available frame is (used only by the map() function)
      Implemented as a counter in DISK
      init created a tracker file with value "1"
      Value gets incremented each time it is retrieved
      ** Value is NOT incremented if peeked

Why we chose this architecture
  Esialy increase sizeof RAM
      Increase the sizeof RAM 64 -> 64 + x*16 where x is any number that will be added to number of frames on RAM from DISK
  Esialy increase sizeof pagetable from 16 to 32 bytes
      Increases the max number of processes from 4 to 8
      32 -> 48 would increase the max number of processes from 8 to 12
      48 -> 56 would increase the max number of processes from 12 to 14 (which is the max supported by this architecture)
