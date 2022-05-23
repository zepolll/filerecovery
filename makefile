cc=gcc
CFLAGS=-I.
main: cl_getMftAdd.o mainGetMft.o nameInfo.o
	$(CC) -o main cl_getMftAdd.o mainGetMft.o nameInfo.o
clean:
	rm *.o main
