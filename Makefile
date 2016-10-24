CFLAGS=	-pipe -g -O2 -Wall -Werror

all:	evq.o

evq.o:	evq.h evq_epoll.c

clean:
	rm -f *.o
