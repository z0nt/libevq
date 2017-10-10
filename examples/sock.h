/*-
 * Copyright (c) 2016 Andrey Zonov
 * All rights reserved.
*/

#ifndef __SOCK_H__
#define __SOCK_H__

int sock_listen(const char *ip, unsigned short port);
int sock_accept(int sockfd);

#endif /* !__SOCK_H__ */
