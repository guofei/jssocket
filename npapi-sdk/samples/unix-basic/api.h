#ifndef api_h_
#define api_h_
#include <unistd.h>
#include <stdio.h>

extern void DebugMsg(char*);
extern int tcp_connect(char *ser, int portno);
extern int tcp_acc_port( int portno, int ip_version );
extern ssize_t readn(int fd, void *vptr, size_t n);
extern ssize_t readline(int fd, void *vptr, size_t maxlen);
extern ssize_t writen(int fd, const void *vptr, size_t n);
extern int fdopen_sock( int sock, FILE **inp, FILE **outp );

#endif
