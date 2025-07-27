CC=gcc
CFLAGS=-Wall -Werror
OBJ = main.o
EXEC = main
DEPS = ram.c pagetable.c tracker.c string.c disk.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXEC)

main: main.o $(DEPS)
	$(CC) -pthread $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(EXEC)