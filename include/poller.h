#ifndef POLLER_H
#define POLLER_H

#define POLLER_MAX_EVENTS 1024
#define POLLER_TIMEOUT -1 // nondeterministic waiting time

struct handler; // forward declaration
typedef void (*callback)(struct handler *this);


struct handler {
  callback handle;
  void *closure;
};

struct epoll_desc {
  int epollfd;
  int listenfd;
  struct epoll_event events[POLLER_MAX_EVENTS];
};


void epoll_loop(struct epoll_desc *desc);

void register_handler(int epollfd, int sockfd, int events, struct handler *handler);

#endif
