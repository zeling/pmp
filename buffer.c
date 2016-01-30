#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "buffer.h"

#define INITIAL_BUF_SIZE 1024
#define EXTRA_BUF 65536

static void reset_rp(struct buffer *buffer);

struct buffer *new_buffer()
{
  char *buf = (char *)malloc(INITIAL_BUF_SIZE*sizeof(char));
  struct buffer *buffer = (struct buffer *)malloc(sizeof(struct buffer));
  buffer->rp = buffer->wp = buffer->lo = buf;
  buffer->hi = buffer->lo + INITIAL_BUF_SIZE;
  return buffer;
}

void free_buffer(struct buffer *buffer)
{
  free(buffer->lo);
  free(buffer);
}

char *retrieve(struct buffer *buffer, size_t size)
{
  char *t = buffer->rp + size;
  if (buffer->hi > t) {
    return buffer->rp = t;
  } else {
    return (char *)-1;
  }
}

ssize_t read_fd(int fd, struct buffer *buffer)
{
  char buf[EXTRA_BUF]; /* stack buffers have relatively low costs to allocate and free */
  size_t available = buffer->hi - buffer->wp;
  size_t pro = buffer->rp - buffer->lo;
  if (available < 128) reset_rp(buffer), available += pro;
  struct iovec iov[2];
  iov[0].iov_base = buffer->wp;
  iov[0].iov_len = buffer->hi - buffer->wp;
  iov[1].iov_base = buf;
  iov[1].iov_len = EXTRA_BUF;
  ssize_t ret = readv(fd, iov, available < EXTRA_BUF ? 2 : 1);
  if (ret < 0) {
    perror("readv");
  } else if ((size_t)ret <= available) {
    buffer->wp += ret;
  } else {
    size_t bytes_to_copy = ret - available;
    buffer->wp = buffer->hi;
    ensure_wsize(buffer, bytes_to_copy);
    memcpy(buffer->wp, buf, bytes_to_copy);
  }
  return ret;
}

static void reset_rp(struct buffer *buffer)
{
  int offset = buffer->rp - buffer->lo;
  /* high to low address, no need to use memmove */
  memcpy(buffer->lo, buffer->rp, buffer->wp - buffer->rp);
  buffer->rp = buffer->lo;
  buffer->wp -= offset;
}

void ensure_wsize(struct buffer *buffer, size_t w_size)
{
  int pro = buffer->rp - buffer->lo;
  int epi = buffer->hi - buffer->wp;
  if (epi >= (int)w_size) return;
  /* inplace */
  reset_rp(buffer);
  if (pro + epi < (int)w_size) {
    size_t alloc_size = w_size + buffer->wp - buffer->lo;
    char *buf = (char *)malloc(alloc_size);
    memcpy(buf, buffer->lo, buffer->wp - buffer->lo);
    int rp_offset = buffer->rp - buffer->lo;
    int wp_offset = buffer->wp - buffer->lo;
    free(buffer->lo);
    buffer->lo = buf;
    buffer->hi = buf + alloc_size;
    buffer->rp = buf + rp_offset;
    buffer->wp = buf + wp_offset;
  }
}

ssize_t write_fd(int fd, struct buffer *buffer, size_t size)
{
  int n, ns = size;
  do {
    n = write(fd, buffer->rp, size);
    if (n < 0) {
      perror("write");
      return n;
    }
    retrieve(buffer, n);
    ns -= n;
  } while (ns > 0);
  return size;
}
