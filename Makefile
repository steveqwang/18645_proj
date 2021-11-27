CC = gcc
CFLAGS = -O3 -std=c99

all: 
	$(CC) $(CFLAGS) $(OBJS) ga.c  -o a.x
run:
	./a.x

clean:
	rm -f *.x *~ *.o
