CC = gcc
CFLAGS = -O3 -std=c99

all: 
	$(CC) $(CFLAGS) $(OBJS) ga.c  -o ga.x
	$(CC) $(CFLAGS) $(OBJS) ga_old.c  -o ga_old.x
run:
	./a.x

clean:
	rm -f *.x *~ *.o
