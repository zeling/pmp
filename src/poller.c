#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "poller.h"

static void accept_handler(struct handler *this);

void epoll_loop(struct epoll_desc *desc)
{
  struct epoll_event ev;
  assert(desc->listenfd > 0);

  desc->epollfd = epoll_create1(0);
  if (desc->epollfd < 0) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }


  while(1) {
    int ev_num = epoll_wait(desc->epollfd, desc->events, POLLER_MAX_EVENTS, POLLER_TIMEOUT);

    for (int i = 0; i != ev_num; ++i) {
      if (desc->events[i].events & EPOLLRDHUP) {
        /* peer shutdown */
      }
      struct handler *handler = (struct handler *)desc->events[i].data.ptr;
      assert(handler != NULL);
      handler->handle(handler);
    }
  }
}


static void accept_handler(struct handler *this)
{
  struct epoll_desc *desc = (struct epoll_desc *)this->closure;
}

void register_handler(int epollfd, int sockfd, int events, struct handler *handler)
{
  struct epoll_event ev;
  ev.data.ptr = handler;
  ev.events = events;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
}

void remove_handler(int epollfd, int sockfd)
{
  /* FIXME: This will cause potential memory leak. Perhaps we finally still need one eventloop. */
  epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
}
