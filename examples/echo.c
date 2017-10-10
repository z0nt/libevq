/*-
 * Copyright (c) 2017 Andrey Zonov
 * All rights reserved.
*/

#include <sys/queue.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evq.h"
#include "sock.h"

struct state;
typedef int (*handler_t)(struct state *);
static int acceptor(struct state *);
static int reader(struct state *);
static int writer(struct state *);

struct state {
	handler_t	handler;
	int		fd;
};

struct io {
	size_t		rpos;
	size_t		wpos;
	char		buf[1024];
};

struct client_state {
	struct state	state;
	struct io	io;
	LIST_ENTRY(client_state) link;
};

#define to_client_state(s) ((void *)(s) - offsetof(struct client_state, state))

static evq_t evq;
static LIST_HEAD(, client_state) clients = LIST_HEAD_INITIALIZER(clients);

static void
cleanup(struct state *state)
{
	struct client_state *client = to_client_state(state);

	close(state->fd);
	LIST_REMOVE(client, link);
	free(client);
}

static int
acceptor(struct state *state)
{
	int fd;
	struct client_state *client;

	fd = sock_accept(state->fd);
	if (fd == -1) {
		warn("sock_accept failed");
		return (0);
	}

	client = malloc(sizeof(*client));
	if (client == NULL) {
		warn("malloc");
		close(fd);
		return (0);
	}
	client->state.handler = reader;
	client->state.fd = fd;
	client->io.wpos = 0;
	LIST_INSERT_HEAD(&clients, client, link);
	if (evq_add_read(evq, fd, &client->state) != 0) {
		warn("evq_add_read failed");
		close(fd);
		free(client);
	}

	return (0);
}

static int
reader(struct state *state)
{
	struct client_state *client = to_client_state(state);
	struct io *io = &client->io;
	ssize_t n;

	n = read(state->fd, io->buf, sizeof(io->buf));
	if (n == -1) {
		warn("read");
		if (errno == EAGAIN || errno == EINTR)
			return (0);
		return (errno);
	}
	if (n == 0)
		return (ENOTCONN);

	io->rpos = n;
	state->handler = writer;
	return (evq_set_write(evq, state->fd, state) != 0);
}

static int
writer(struct state *state)
{
	struct client_state *client = to_client_state(state);
	struct io *io = &client->io;
	ssize_t n;

	n = write(state->fd, io->buf + io->wpos, io->rpos - io->wpos);
	if (n == -1) {
		warn("write");
		if (errno == EAGAIN || errno == EINTR)
			return (0);
		return (errno);
	}
	if (n == 0)
		return (0);

	io->wpos += n;
	if (io->wpos != io->rpos)
		return (0);

	io->wpos = 0;
	state->handler = reader;
	return (evq_set_read(evq, state->fd, state) != 0);
}

static void
sig_quit(int sig)
{
}

int
main(void)
{
	int fd, done;
	struct state server_state, *state;

	signal(SIGQUIT, sig_quit);
	signal(SIGPIPE, SIG_IGN);

	evq = evq_create(0);
	if (evq == NULL)
		err(1, "evq_create failed");

	fd = sock_listen("127.0.0.1", 55555);
	if (fd == -1)
		err(1, "sock_listen failed");

	server_state.handler = acceptor;
	server_state.fd = fd;
	if (evq_add_read(evq, fd, &server_state) != 0)
		err(1, "evq_add_read failed");

	while (evq_wait(evq, -1) != -1) {
		while ((state = evq_next(evq)) != NULL) {
			done = state->handler(state);
			if (done)
				cleanup(state);
		}
	}

	while (!LIST_EMPTY(&clients))
		cleanup(&LIST_FIRST(&clients)->state);
	close(fd);
	evq_destroy(evq);

	return (0);
}
