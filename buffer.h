#ifndef _BUFFER_H
#define _BUFFER_H

struct buffer {
  char *lo;
  char *hi;
  char *rp; /* point to the memory
               which is readable */
  char *wp; /* point to the memory
               which is writable; */
};

struct buffer *new_buffer();
void free_buffer(struct buffer *buffer);
char *retreive(struct buffer *buffer, size_t size);
ssize_t read_fd(int fd, struct buffer *buffer);
static void reset_rp(struct buffer *buffer);
void ensure_wsize(struct buffer *buffer, size_t w_size);

#endif
