#ifndef _BUFFER_H
#define _BUFFER_H

#include <stdint.h>

struct buffer {
    uint8_t *hi;
    uint8_t *rp; /* point to the memory which is readable */
    uint8_t *wp; /* point to the memory which is writable; */
    uint8_t lo[0];
};

struct buffer *new_buffer(size_t size);
void free_buffer(struct buffer *buffer);
char *retrieve(struct buffer *buffer, size_t size);
ssize_t read_fd(int fd, struct buffer *buffer);
ssize_t write_fd(int fd, struct buffer *buffer, size_t size);
void ensure_wsize(struct buffer *buffer, size_t w_size);

#endif
