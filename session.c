#include "session.h"
#include <stdlib.h>

int init_session(struct proxy_session *session, int fd1, int fd2)
{
  if (pipe(session->pipefds) < 0) {
    perror("create pipe when initializing session");
    return -1;
  }
  session->sockfds[0] = fd1;
  session->sockfds[1] = fd2;
  return 0;
}

int close_session(struct proxy_session *session) {
  close(session->sockfds[0]);
  close(session->sockfds[1]);
  close(session->pipefds[0]);
  close(session->pipefds[1]);
  free(session);
  return 0;
}
