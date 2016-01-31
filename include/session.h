#ifndef _SESSION_H
#define _SESSION_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct proxy_session {
  int sockfds[2]; /* sockfds[0] is the socket between client
                     and proxy while sockfds[1] is the one
                     between proxy and the server */
  int pipefds[2]; /* pipefds[0] is the read end of the pipe
                     pipefds[1] is the write end of the pipe */
};

int init_session(struct proxy_session *session, int fd1, int fd2);
int close_session(struct proxy_session *session);

#endif
