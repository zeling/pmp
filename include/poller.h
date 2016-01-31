#ifndef POLLER_H
#define POLLER_H

#define POLLER_MAX_EVENTS 1024
#define POLLER_TIMEOUT -1 // nondeterministic waiting time

struct handler; // forward declaration
typedef void (*callback)(struct handler *this);


struct handler {
  callback cb;
  void *closure;
};

struct epoll_desc {
  int epollfd;
  struct handler handlers[12];
  struct epoll_event events[POLLER_MAX_EVENTS];
};


void poll_loop(int epollfd, struct epoll_event *events,
               callback rc, callback wc);

void epoll_loop(struct epoll_desc *desc);

#endif
