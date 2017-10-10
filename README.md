## Event Queue Library

### Synopsis

```c
evq_t evq_create(int nevents);
void evq_destroy(evq_t evq);

int evq_add_read(evq_t evq, int fd, void *data);
int evq_add_write(evq_t evq, int fd, void *data);
int evq_set_read(evq_t evq, int fd, void *data);
int evq_set_write(evq_t evq, int fd, void *data);
int evq_del(evq_t evq, int fd, void *data);

int evq_wait(evq_t evq, int msec);
void *evq_next(evq_t evq);
```
### Description
The `evq_create()` call creates an event queue instance and it is used as a
first argument to other calls.  The _nevents_ argument determines how much
memory will be allocated internally.  Default value is used if zero or negative
value is provided.  `evq_destroy()` should be called to release resources.

The `evq_add_read()` and `evq_add_write()` calls register file descriptor _fd_
for read or write events respectively.

The `evq_wait()` call will wait for new events at least _msec_ milliseconds.
It returns positive value if one or more events are triggered.  The _data_
asssociated with the event is returned by `evq_next()` call.
