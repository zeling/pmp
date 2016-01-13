#ifndef POLLER_H
#define POLLER_H

#include <sys/epoll.h>

#define POLLER_MAX_EVENTS 1024
#define POLLER_TIMEOUT -1 // nondeterministic waiting time

typedef int(*callback)(struct epoll_event *);

void poll_loop(int epollfd, struct epoll_event *events,
               callback rc, callback wc);

#endif
