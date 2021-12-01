CC = gcc
CFLAGS = -O3 -std=c99

all: 
	gcc -O3 ga.c  -o ga.x -std=c99 -fopenmp
	gcc -O3 ga_old.c  -o ga_old.x -std=c99 -fopenmp

run:
	./ga.x

clean:
	rm -f *.x *~ *.o
