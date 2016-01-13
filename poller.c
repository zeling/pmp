#include "poller.h"

void poll_loop(int epollfd, struct epoll_event *events,
               callback rc, callback wc)
{
  while (1) {
    int i, ev_num = epoll_wait(epollfd, events,
                               POLLER_MAX_EVENTS, -1);
    for (i = 0; i < ev_num; i++) {
      if (events[i].events & EPOLLIN) {
        if (rc) rc(&events[i]);
      } else if (events[i].events & EPOLLOUT) {
        if (wc) wc(&events[i]);
      }
    }
  }
}

