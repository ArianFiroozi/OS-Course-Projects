CC = gcc
CFLAGS = -Wall -Wextra

all:	r c s

c:	client.c lib.h
	$(CC) $(CFLAGS) client.c -o client.out

r:	restaurant.c lib.h
	$(CC) $(CFLAGS) restaurant.c -o restaurant.out

s:	supplier.c lib.h
	$(CC) $(CFLAGS) supplier.c -o supplier.out


clean:
	rm -f c r s *.out