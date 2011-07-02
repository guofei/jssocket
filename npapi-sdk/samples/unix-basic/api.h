#ifndef api_h_
#define api_h_
#include <unistd.h>

extern void DebugMsg(char*);
extern int tcp_connect(char *ser, int portno);
extern ssize_t writen(int fd, const void *vptr, size_t n);
extern ssize_t readline(int fd, void *vptr, size_t maxlen);

#endif
