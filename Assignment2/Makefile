obj-m+=processList.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) userProgram.c -o userProgram
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm userProgram
