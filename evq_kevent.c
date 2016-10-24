/*-
 * Copyright (c) 2016 Andrey Zonov
 * All rights reserved.
*/

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evq.h"

struct evq_opaque {
	int kqfd;
	int nchanges;
	int nevents;
	struct kevent *current;
	struct kevent *end;
	struct kevent *eventlist;
	struct kevent changelist[];
};

evq_t
evq_create(int nevents)
{
	evq_t evq;

	if (nevents <= 0)
		nevents = 512;

	evq = malloc(sizeof(*evq) + nevents * 3 * sizeof(struct kevent));
	if (evq == NULL)
		return (NULL);
	evq->kqfd = kqueue();
	if (evq->kqfd == -1) {
		free(evq);
		return (NULL);
	}
	evq->nchanges = 0;
	evq->nevents = nevents;
	evq->eventlist = evq->changelist + nevents * 2;
	evq->current = evq->eventlist;
	evq->end = evq->current;

	return (evq);
}

void
evq_destroy(evq_t evq)
{

	close(evq->kqfd);
	free(evq);
}

#ifdef DEBUG
static void
evq_dump(evq_t evq, struct kevent *kev, int in)
{
	printf("%s evq: %p, ident: %4ld, filter: %4hd, flags: 0x%04hx, "
	    "fflags: %2u, data: %8ld, udata: %p\n",
	    in ? "==>" : "<==", evq, kev->ident, kev->filter, kev->flags,
	    kev->fflags, kev->data, kev->udata);
}
#else
#define evq_dump(evq, kev, in)
#endif

static inline int
kevent_push(evq_t evq)
{

	if (evq->nchanges == evq->nevents * 2) {
		if (kevent(evq->kqfd, evq->changelist, evq->nchanges,
		    NULL, 0, NULL) == -1) {
			abort(); /* XXX */
			return (-1);
		}
		evq->nchanges = 0;
	}
	return (0);
}

static inline void
kevent_set(evq_t evq, int fd, int filter, int flags, void *data)
{
	struct kevent *kev;

	kev = &evq->changelist[evq->nchanges++];
	EV_SET(kev, fd, filter, flags, 0, 0, data);
	evq_dump(evq, kev, 0);
}

int
evq_add_read(evq_t evq, int fd, void *data)
{

	if (kevent_push(evq) == -1)
		return (-1);
	kevent_set(evq, fd, EVFILT_READ,  EV_ADD | EV_ENABLE,  data);
	kevent_set(evq, fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, data);
	return (0);
}

int
evq_add_write(evq_t evq, int fd, void *data)
{

	if (kevent_push(evq) == -1)
		return (-1);
	kevent_set(evq, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE,  data);
	kevent_set(evq, fd, EVFILT_READ,  EV_ADD | EV_DISABLE, data);
	return (0);
}

int
evq_set_read(evq_t evq, int fd, void *data)
{

	if (kevent_push(evq) == -1)
		return (-1);
	kevent_set(evq, fd, EVFILT_READ,  EV_ENABLE,  data);
	kevent_set(evq, fd, EVFILT_WRITE, EV_DISABLE, data);
	return (0);
}

int
evq_set_write(evq_t evq, int fd, void *data)
{

	if (kevent_push(evq) == -1)
		return (-1);
	kevent_set(evq, fd, EVFILT_WRITE, EV_ENABLE,  data);
	kevent_set(evq, fd, EVFILT_READ,  EV_DISABLE, data);
	return (0);
}

int
evq_del(evq_t evq, int fd, void *data)
{

	if (kevent_push(evq) == -1)
		return (-1);
	kevent_set(evq, fd, EVFILT_READ,  EV_DELETE, data);
	kevent_set(evq, fd, EVFILT_WRITE, EV_DELETE, data);
	return (0);
}

int
evq_wait(evq_t evq, int msec)
{
	const struct timespec ts = {
		.tv_sec = msec / 1000,
		.tv_nsec = (msec % 1000) * 1000000
	};
	int nevents;

	nevents = kevent(evq->kqfd, evq->changelist, evq->nchanges,
	    evq->eventlist, evq->nevents, msec < 0 ? NULL : &ts);
	evq->nchanges = 0;
	if (nevents > 0) {
#ifdef DEBUG
		printf("kevent() nevents: %d\n", nevents);
#endif
		evq->current = evq->eventlist;
		evq->end = evq->current + nevents;
	}

	return (nevents);
}

void *
evq_next(evq_t evq)
{
	struct kevent *kev;

	while (evq->current != evq->end) {
		kev = evq->current;
		evq->current++;
		evq_dump(evq, kev, 1);
		if (kev->flags & EV_ERROR)
			continue;
		return (kev->udata);
	}
	return (NULL);
}
