/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Andrew Carnegie, ac00@cs.cmu.edu 
 *     Harry Q. Bovik, bovik@cs.cmu.edu
 *
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <assert.h>
#include <errno.h>

#include "session.h"

#define MAXLINE 8192
#define LISTENQ 1024
#define MAXEVENTS 4096


//struct proxy_session {
//int cli_pro; // sockfd between client and proxy
//int pro_serv; // sockfd between proxy and client
//int pipe[2];
//char* urlinreq;
//};

struct data {
  int desc;
  struct proxy_session *session;
};


struct context {
  int desc; /* if it is a negative number,
             its abs indicates its size to
             write, otherwise its the
             descriptor of the session */
  struct proxy_session *session;
  char *buf;
};

struct data_w {
  int fd;
  char *buf; /* do not delete buf, because it is generally on the stack */
};


/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

struct epoll_event events[MAXEVENTS];
struct epoll_event ev;
/*
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
  int ret;
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = htonl(INADDR_ANY),
    .sin_port = htons(atoi(argv[1]))
  };

  /* Check arguments */
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
    exit(0);
  }

  int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (listenfd < 0) {
    perror("socket");
  }

  int optval = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                 (const void *)&optval, sizeof(int)) < 0) {
    perror("setsockopt");
  }

  if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
  }

  if (listen(listenfd, LISTENQ) < 0) {
    perror("listen");
  }


  int epollfd = epoll_create1(0);
  if (epollfd < 0) {
    perror("epoll_create1");
    goto clolisfd;
  }

  ev.events = EPOLLIN;
  ev.data.ptr = NULL;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
  int ev_num;
  int size;
  char *buf, *rp;
  //char buf[MAXLINE], *rp;
  struct proxy_session *session;
  struct context* context;
  while (1) {
    ev_num = epoll_wait(epollfd, events, MAXEVENTS, -1);
    int n, i, sockfd;
    for (i = 0; i < ev_num; i++) {
      if (events[i].data.ptr == NULL) {
        int client = accept4(listenfd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client < 0) {
          perror("accept4");
          goto cloepoll;
        }
        context = (struct context *)malloc(sizeof(struct context));
        session = (struct proxy_session *)malloc(sizeof(struct proxy_session));
        init_session(session, client, -1);
        context->desc = 0;
        context->session = session;
        context->buf = NULL;
        ev.data.ptr = context;
        ev.events = EPOLLIN;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev);
      } else if (events[i].events & EPOLLIN) {
        context = (struct context *)events[i].data.ptr;
        session = context->session;
        if (context->desc == 0) {
          if (session->sockfds[1] == -1) {
            sockfd = session->sockfds[0];
            /* We got here for the first time */
            char method[10], url[MAXLINE], version[10];
            buf = malloc(4096);
            char *rp = buf;
            do {
              n = read(sockfd, rp, MAXLINE);
              if (n < 0) {
                perror("read");
                if (errno == ECONNRESET) {
                  epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
                  epoll_ctl(epollfd, EPOLL_CTL_DEL,
                            session->sockfds[1], NULL);
                  close_session(session);
                  free(context);
                }
                goto cleanup;
              }
              rp += n;
            } while (sscanf(buf, "%s %s %s", method, url, version) != 3);
            size = rp - buf;
            char hostname[MAXLINE], pathname[MAXLINE];
            int port;
            parse_uri(url, hostname, pathname, &port);

            struct hostent *hp = gethostbyname(hostname);
            if (hp == NULL) {
              fprintf(stderr, "gethostbyaddr");
              goto cleanup;
            }
            struct sockaddr_in serv_addr = {
              .sin_family = AF_INET,
              .sin_port = htons(port)
            };
            bcopy((char *)hp->h_addr_list[0],
                  (char *)&serv_addr.sin_addr.s_addr,
                  hp->h_length);

            int req = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
            session->sockfds[1] = req;
            context = (struct context *)malloc(sizeof(struct context));
            context->session = session;
            context->buf = NULL;
            if (connect(req, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
              if (errno == EINPROGRESS) {
                context->buf = buf;
                context->desc = size;
                ev.events = EPOLLOUT;
                ev.data.ptr = context;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, req, &ev);
                continue;
              }
              perror("connect");
              goto cleanup;
            }
            context->desc = 1;
            ev.events = EPOLLIN;
            ev.data.ptr = context;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, req, &ev);
            rp = buf;
            do {
              n = write(req , rp, size);
              size -= n;
              rp += n;
            } while (size > 0);
          } else {
            /* the session is already created */
            splice(session->sockfds[0], NULL, session->pipefds[1], NULL, 65535, SPLICE_F_MORE | SPLICE_F_NONBLOCK);
            splice(session->pipefds[0], NULL, session->sockfds[1], NULL, 65535, SPLICE_F_MORE | SPLICE_F_NONBLOCK);
          }
        } else {
          splice(session->sockfds[1], NULL, session->pipefds[1], NULL, 65535, SPLICE_F_MORE | SPLICE_F_NONBLOCK);
          splice(session->pipefds[0], NULL, session->sockfds[0], NULL, 65535, SPLICE_F_MORE | SPLICE_F_NONBLOCK);
        }
      } else if (events[i].events & EPOLLOUT) {
        context = (struct context *)events[i].data.ptr;
        session = context->session;
        buf = context->buf;
        if (buf != NULL) {
          size = context->desc;
          rp = buf;
          do {
            n = write(session->sockfds[1], rp, size);
            size -= n;
            rp += n;
          } while (size > 0);
        }
        context->desc = 1;
        free(buf);
        context->buf = NULL;
        ev.events = EPOLLIN;
        ev.data.ptr = context;
        epoll_ctl(epollfd, EPOLL_CTL_MOD, session->sockfds[1], &ev);
      }
    }
  }

  exit(EXIT_SUCCESS);

  /* resource handling */

 cleanup:
 cloepoll:
  ret = close(epollfd);
  if (ret < 0) {
    perror("close epollfd");
  }
 clolisfd:
  ret = close(listenfd);
  if (ret < 0) {
    perror("close listenfd");
  }
  exit(EXIT_FAILURE);
}


/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
  char *hostbegin;
  char *hostend;
  char *pathbegin;
  int len;

  if (strncasecmp(uri, "http://", 7) != 0) {
    hostname[0] = '\0';
    return -1;
  }
       
  /* Extract the host name */
  hostbegin = uri + 7;
  hostend = strpbrk(hostbegin, " :/\r\n\0");
  len = hostend - hostbegin;
  strncpy(hostname, hostbegin, len);
  hostname[len] = '\0';
    
  /* Extract the port number */
  *port = 80; /* default */
  if (*hostend == ':')   
    *port = atoi(hostend + 1);
    
  /* Extract the path */
  pathbegin = strchr(hostbegin, '/');
  if (pathbegin == NULL) {
    pathname[0] = '\0';
  }
  else {
    pathbegin++;	
    strcpy(pathname, pathbegin);
  }

  return 0;
} /*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
                      char *uri, int size)
{
  time_t now;
  char time_str[MAXLINE];
  unsigned long host;
  unsigned char a, b, c, d;

  /* Get a formatted time string */
  now = time(NULL);
  strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

  /* 
   * Convert the IP address in network byte order to dotted decimal
   * form. Note that we could have used inet_ntoa, but chose not to
   * because inet_ntoa is a Class 3 thread unsafe function that
   * returns a pointer to a static variable (Ch 13, CS:APP).
   */
  host = ntohl(sockaddr->sin_addr.s_addr);
  a = host >> 24;
  b = (host >> 16) & 0xff;
  c = (host >> 8) & 0xff;
  d = host & 0xff;


  /* Return the formatted log entry string */
  sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
}


