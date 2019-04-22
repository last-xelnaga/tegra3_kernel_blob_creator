
all: kernel_blob_creator

kernel_blob_creator: main.o
	gcc main.o -o kernel_blob_creator

main.o: main.c Makefile
	gcc -c -Wall main.c

clean:
	rm -rf *.o kernel_blob_creator
