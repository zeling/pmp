#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "buffer.h"


void display(struct buffer *buffer) {
  char newline = '\n';
  char escape_start[] = "\e[1;45m";
  char escape_end[] = "\e[0m";
  write(2, &newline, 1);
  for (char *p = buffer->lo; p != buffer->hi; p++) {
    if (__builtin_expect(p == buffer->rp || p == buffer->wp, 0)) {
      write(2, escape_start, 7);
    }
    write(2, p, 1);
    if (__builtin_expect(p == buffer->rp || p == buffer->wp, 0)) {
      write(2, escape_end, 4);
    }
  }
}

int main() {
  struct buffer *buffer = new_buffer(1024);
  int fd = open("random.txt", O_RDONLY);
  int fd2 = open("random.txt", O_RDONLY);
  read_fd(fd, buffer);
  display(buffer);
  write_fd(2, buffer, 2);
  display(buffer);
  write_fd(2, buffer, 2);
  display(buffer);
  read_fd(fd2, buffer);
  display(buffer);
  write_fd(2, buffer, 2);
  display(buffer);
  write_fd(2, buffer, 2);
  display(buffer);
  write_fd(2, buffer, 3);
  display(buffer);
  free_buffer(buffer);
  close(fd);
  close(fd2);
}


