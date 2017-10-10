CFLAGS=	-pipe -g -O2 -Wall -Werror

.PHONY:	examples
all:	evq.o examples

examples:
	$(MAKE) -C examples

evq.o:	evq.h evq_epoll.c evq_kevent.c

clean:
	rm -f *.o
	$(MAKE) -C examples clean
