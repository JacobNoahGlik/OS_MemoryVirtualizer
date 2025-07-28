# OS Memory Virtualizer

**OS_MemoryVirtualizer** is a lightweight, dependable memory management system that simulates a basic virtual memory architecture. Initially developed as a university project, this version expands on the original design by adding missing features, such as process reaping (`kill`), and improving both code structure and debugging support.

It demonstrates a deep understanding of low-level OS concepts such as virtual-to-physical address translation, paging, memory protection, and disk swapping — all built from scratch in C with minimal dependencies.


## Build & Run

### Make Commands
```bash
make        # Compiles the project
make clean  # Removes build artifacts
````

### Run the Program

```bash
./main -debug_pagetable -debug_pagetable_b -debug_ram
```

---

<br>

## Debugging Flags

You can enable optional runtime visualizations using the following flags:

* `-debug_pagetable`
  Show the page table in a readable format (decimal values).

* `-debug_pagetable_b`
  Show the binary representation of the page table (bit-by-bit).

* `-debug_ram`
  Dump the full contents of RAM in hex after every instruction.

* All flags can be combined (e.g., `./main -debug_pagetable -debug_ram`)

* `-help` or `-h`
  Prints all supported flags and exits.

---

<br>

## Legal Input Format

Each instruction must follow this format:

```text
<ProcessID>,<Instruction>,<VirtualByte>,<Value>
```

### Example Input

```text
0,map,0,1
0,store,0,42
0,load,0,0
0,kill,0,0
1,map,0,1
0,exit,0,0
```

### Supported Instructions

| Instruction | Description                                                                                          |
| ----------- | ---------------------------------------------------------------------------------------------------- |
| `map`       | Maps a virtual page to RAM or disk with a specified protection bit (0 = read-only, 1 = read/write).  |
| `store`     | Writes a value to a mapped virtual address if writable.                                              |
| `load`      | Loads a value from a mapped virtual address.                                                         |
| `kill`      | Clears the page table entry for a given process, freeing its slot for reuse. *(New in this version)* |
| `exit`      | Terminates the simulation and deallocates all memory/disk/tracker files.                             |

---

<br>

## Architecture Overview

### Memory Layout

* **64 bytes of RAM**

  * 16 bytes reserved for the page table (Frame 0)
  * 3 × 16-byte RAM frames (Frame 1–3) for active data pages
* **208 bytes of Disk**

  * 13 × 16-byte frames used to swap out virtual pages (Frame 4–62)

### Page Table Format

* 4 rows total, each 4 bytes long
* Bit-packed format:

  * 4 bits: Process ID (`0000` = unused, `0001–1111` = PID + 1)
  * 28 bits: 4 × (6-bit physical frame + 1-bit writeable flag)

### Page Swapping

* Pages not currently in RAM are swapped in from disk
* Least idle RAM frame is selected for eviction using a tracker
* Swapped-out RAM frames are saved to disk and vice versa

---

<br>

## Disk and Tracker

* A `disk` file is created at runtime to simulate page storage
* Each 16-byte frame is written as a line of hex (2 hex characters per byte)
* A `nextpage.tracker` file tracks RAM frame idleness:

  * Operates like a stack with values "1 2 3"
  * Updated on every page access or swap

---

<br>

## Sample Run

```text
Instruction? 0,map,0,1
Mapped virtual address 0 of PID 0 to frame 1 (writable)

Instruction? 0,store,0,99
Stored value 99 at virtual address 0 (physical frame 1)

Instruction? 0,load,0,0
The value 99 is at virtual address 0 (physical frame 1)

Instruction? 0,kill,0,0
Process 0 killed and its page table entry cleared.

Instruction? 1,map,0,1
Mapped virtual address 0 of PID 1 to frame 2 (writable)
```

---

<br>

## What’s New in This Version

* ✅ **`kill` command** to free up page table slots on demand
* ✅ Refactored instruction handling loop (easier to extend)
* ✅ Typed constants for cleaner page table initialization
* ✅ Improved debug output with consistent formatting
* ✅ Professional README and code documentation

---

<br>

## 🔭 Future Work

This memory virtualizer is intentionally minimal — but extensible. Future enhancements could include:

*  **ASM-like Instruction Runner**
  Build a companion project that executes simple assembly files using this memory system — simulate up to 4 concurrent processes accessing isolated virtual spaces.

*  **Enhanced Debugging Tools**
  Add a curses-based TUI or graphical visualizer to show page table, RAM, and disk mappings live.

*  **Instruction Logging**
  Optionally log each instruction’s effect on RAM and page table to a file or stdout.

*  **Test Harness**
  Create regression tests using predefined input scripts and expected memory state dumps.

---

<br>

##  File Structure

```text
main.c               // Instruction input and dispatch
pagetable.c/.h       // Page table layout and manipulation
ram.c/.h             // RAM allocator and hex dumper
tracker.c/.h         // RAM idleness + next page tracking
disk.c/.h            // Disk-backed page swap system
string.c/.h          // Lightweight string utils
Makefile
README.md
```


<br>
