CFLAGS=	-pipe -g -O2 -Wall -Werror -I..

all:	bench

bench:	bench.o ../evq.o

bench.o: CFLAGS+= -DWITH_LIBEVQ

clean:
	rm -f *.o
	rm -f bench
