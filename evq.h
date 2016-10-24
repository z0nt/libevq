/*-
 * Copyright (c) 2016 Andrey Zonov
 * All rights reserved.
*/

#ifndef __EVQ_H__
#define __EVQ_H__

typedef struct evq_opaque *evq_t;

evq_t evq_create(int nevents);
void evq_destroy(evq_t evq);

int evq_add_read(evq_t evq, int fd, void *data);
int evq_add_write(evq_t evq, int fd, void *data);
int evq_set_read(evq_t evq, int fd, void *data);
int evq_set_write(evq_t evq, int fd, void *data);
int evq_del(evq_t evq, int fd, void *data);

int evq_wait(evq_t evq, int msec);
void *evq_next(evq_t evq);

#endif /* !__EVQ_H__ */
