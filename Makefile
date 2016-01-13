TEAM = NOBODY
VERSION = 1
HANDINDIR = /afs/cs/academic/class/15213-f02/L7/handin

CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lpthread

OBJS = proxy.o session.o

all: proxy

proxy: $(OBJS) -lpthread

proxy.o: proxy.c
	$(CC) $(CFLAGS) -c proxy.c

session.o: session.c
	$(CC) $(CFLAGS) -c session.c

buffer.o: buffer.c
	$(CC) $(CFLAGS) -c buffer.c

handin:
	cp proxy.c $(HANDINDIR)/$(TEAM)-$(VERSION)-proxy.c

clean:
	rm -f *~ *.o proxy core

