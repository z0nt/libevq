/*-
 * Copyright (c) 2016 Andrey Zonov
 * All rights reserved.
*/

#if defined(__linux__)
#include "evq_epoll.c"
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include "evq_kevent.c"
#else
#error not implemented
#endif
