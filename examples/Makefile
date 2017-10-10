CFLAGS=	-pipe -g -O2 -Wall -Werror -I..

all:	bench echo

bench:	bench.o ../evq.o
echo:	echo.o sock.o ../evq.o

bench.o: CFLAGS+= -DWITH_LIBEVQ
sock.o:	sock.h

clean:
	rm -f *.o
	rm -f bench
	rm -f echo
