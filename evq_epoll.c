/*-
 * Copyright (c) 2016 Andrey Zonov
 * All rights reserved.
*/

#include <sys/epoll.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evq.h"

struct evq_opaque {
	int epfd;
	int nevents;
	struct epoll_event *current;
	struct epoll_event *end;
	struct epoll_event eventlist[];
};

evq_t
evq_create(int nevents)
{
	evq_t evq;

	if (nevents <= 0)
		nevents = 512;

	evq = malloc(sizeof(*evq) + nevents * sizeof(struct epoll_event));
	if (evq == NULL)
		return (NULL);
	evq->epfd = epoll_create(nevents);
	if (evq->epfd == -1) {
		free(evq);
		return (NULL);
	}
	evq->nevents = nevents;
	evq->current = evq->eventlist;
	evq->end = evq->current;

	return (evq);
}

void
evq_destroy(evq_t evq)
{

	close(evq->epfd);
	free(evq);
}

#ifdef DEBUG
static void
evq_dump(evq_t evq, struct epoll_event *ep, int in)
{
	printf("%s evq: %p, events: 0x%04x, data.ptr: %p\n",
	    in ? "==>" : "<==", evq, ep->events, ep->data.ptr);
}
#else
#define evq_dump(evq, ep, in)
#endif

int
evq_add_read(evq_t evq, int fd, void *data)
{
	struct epoll_event ep;

	ep.events = EPOLLIN;
	ep.data.ptr = data;
	evq_dump(evq, &ep, 0);

	return (epoll_ctl(evq->epfd, EPOLL_CTL_ADD, fd, &ep));
}

int
evq_add_write(evq_t evq, int fd, void *data)
{
	struct epoll_event ep;

	ep.events = EPOLLOUT;
	ep.data.ptr = data;
	evq_dump(evq, &ep, 0);

	return (epoll_ctl(evq->epfd, EPOLL_CTL_ADD, fd, &ep));
}

int
evq_set_read(evq_t evq, int fd, void *data)
{
	struct epoll_event ep;

	ep.events = EPOLLIN;
	ep.data.ptr = data;
	evq_dump(evq, &ep, 0);

	return (epoll_ctl(evq->epfd, EPOLL_CTL_MOD, fd, &ep));
}

int
evq_set_write(evq_t evq, int fd, void *data)
{
	struct epoll_event ep;

	ep.events = EPOLLOUT;
	ep.data.ptr = data;
	evq_dump(evq, &ep, 0);

	return (epoll_ctl(evq->epfd, EPOLL_CTL_MOD, fd, &ep));
}

int
evq_del(evq_t evq, int fd, void *data)
{

	return (epoll_ctl(evq->epfd, EPOLL_CTL_DEL, fd, NULL));
}

int
evq_wait(evq_t evq, int msec)
{
	int nevents;

	nevents = epoll_wait(evq->epfd, evq->eventlist, evq->nevents, msec);
	if (nevents > 0) {
#ifdef DEBUG
		printf("epoll_wait() nevents: %d\n", nevents);
#endif
		evq->current = evq->eventlist;
		evq->end = evq->current + nevents;
	}

	return (nevents);
}

void *
evq_next(evq_t evq)
{
	struct epoll_event *ep;

	if (evq->current != evq->end) {
		ep = evq->current;
		evq->current++;
		evq_dump(evq, ep, 1);
		return (ep->data.ptr);
	}
	return (NULL);
}
