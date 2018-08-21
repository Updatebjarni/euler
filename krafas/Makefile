CFLAGS := -O2 -std=c99 -Wall

tangent: tangent.o mog.o
	gcc -o tangent tangent.o mog.o -lpthread -lasound

bench: bench.o bench-rt.o
	gcc -o bench bench.o -lpthread
	gcc -o bench-rt bench-rt.o -lpthread

%.c: orgel-io.h orgel.h

clean:
	rm -f *.o tangent bench bench-rt orgelperm
