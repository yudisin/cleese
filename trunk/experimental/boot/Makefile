all: KERNEL.BIN

main.o: main.c
	gcc -ffreestanding -c main.c -o main.o

video.o: video.c
	gcc -c video.c -o video.o

ports.o: ports.c
	gcc -c ports.c -o ports.o

kernel.o: main.o video.o ports.o
	ld -Ttext 0xFF80000 -o kernel.o main.o video.o ports.o

KERNEL.BIN: kernel.o
	objcopy -R .note -R .comment -S -O binary kernel.o KERNEL.BIN

clean:
	-rm -f *.o
	-rm -f KERNEL.BIN

