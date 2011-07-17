
/*
	echo-server-fork.c -- 受け取った文字列をそのまま返すサーバ(fork版)
	~yas/syspro-2001/ipc/echo-server-fork.c
	Created on 1997/06/09 19:46:40
*/
#include <stdio.h>
#include <sys/types.h>	/* socket(), time() */
#include <sys/socket.h>	/* socket() */
#include <netinet/in.h>	/* struct sockaddr_in */

extern	void echo_server( int portno );
extern	void echo_reply( int com );
extern	void print_host_port( int portno );
extern	void tcp_peeraddr_print( int com );
extern	int  tcp_acc_port_any( int *portno );
extern	tcp_acc_port( int portno );
extern	ssize_t writen(int fd, const void *vptr, size_t n);
extern	ssize_t readline(int fd, void *vptr, size_t maxlen);

main( int argc, char *argv[] )
{
    int portno ;
	if( argc != 2 )
	{
	    fprintf( stdout,"Usage: %s portno\n",argv[0] );
	    exit( -1 );
	}
	portno = strtol( argv[1],0,10 );
	echo_server( portno );
}

void
echo_server( int portno )
{
    int acc,com ;
    pid_t child_pid ;
	if( portno == 0 )
	{
	    acc = tcp_acc_port_any( &portno );
	}
	else
	{
	    acc = tcp_acc_port( portno );
	}
	if( acc<0 )
	    exit( -1 );
	print_host_port( portno );
	while( 1 )
	{
	    if( (com = accept( acc,0,0 )) < 0 )
	    {
		perror("accept");
		exit( -1 );
	    }
	    tcp_peeraddr_print( com );
	    if( (child_pid=fork()) > 0 ) /* parent */
	    {
		close( com );
	    }
	    else if( child_pid == 0 ) /* parent */
	    {
		close( acc );
		echo_reply( com );
		printf("[%d,%d] connection closed.\n",getpid(),com );
		close( com );
		exit( 0 );
	    }
	    else
	    {
		perror("fork");
		exit( -1 );
	    }  
	}
}

#define	BUFFERSIZE	1024

void
echo_reply( int com )
{
    char line[BUFFERSIZE] ;
    int rcount ;
    int wcount ;

	while( (rcount=readline(com,line,BUFFERSIZE)) > 0 )
	{
	    printf("[%d,%d] read() %d bytes, %s",getpid(),com,rcount,line );
	    fflush( stdout );
	    if( (wcount=writen(com,line,rcount))!= rcount )
	    {
		 perror("write");
		 exit( 1 );
	    }
	}
}

void
print_host_port( int portno )
{
    char hostname[100] ;
	gethostname( hostname,sizeof(hostname) );
	hostname[99] = 0 ;
	printf("run telnet %s %d \n",hostname, portno );
}

void
tcp_peeraddr_print( int com )
{
    struct sockaddr_in addr ;
    int addr_len ;
    union {
	int i ;
	unsigned char byte[4] ;
    } x ;
	addr_len = sizeof( addr );
    	if( getpeername( com, &addr, &addr_len  )<0 )
	{
	    perror("print_peeraddr");
	}
    	x.i = addr.sin_addr.s_addr ;
    	printf("[%d,%d] connection from %d.%d.%d.%d:%d\n",getpid(),com,
	       x.byte[0],x.byte[1],x.byte[2],x.byte[3],
	       ntohs( addr.sin_port ));
}

int
tcp_acc_port( int portno )
{
    struct hostent *hostent ;
    struct sockaddr_in addr ;
    int addr_len ;
    int s ;

	if( (s = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	{
	    perror("socket");
	    return( -1 );
	}

	addr.sin_family = AF_INET ;
	addr.sin_addr.s_addr = INADDR_ANY ;
	addr.sin_port = htons( portno );

	if( bind(s,&addr,sizeof(addr)) < 0 )
	{
	    perror( "bind" );
	    fprintf(stderr,"port number %d is already used. wait a moment or kill another program.\n", portno );
	    return( -1 );
	}
	listen( s, 5 );
	return( s );
}

int
tcp_acc_port_any( int *portnop )
{
    struct hostent *hostent ;
    struct sockaddr_in addr ;
    int len ;
    int s ;

	if( (s = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	{
	    perror("socket");
	    return( -1 );
	}

	addr.sin_family = AF_INET ;
	addr.sin_addr.s_addr = INADDR_ANY ;
	addr.sin_port = 0 ;

	if( bind(s,&addr,sizeof(addr)) < 0 )
	{
	    perror( "bind" );
	    return( -1 );
	}

	len = sizeof( addr );
	if( getsockname( s, &addr, &len )< 0 )
	{
	    perror("getsockname");
	    exit( -1 );
	}
	*portnop = ntohs( addr.sin_port );
	listen( s, 5 );
	return( s );
}

/* 
W.リチャード・スティーブンス著、篠田陽一訳:
"UNIXネットワークプログラミング第２版 Vol.1 ネットワークAPI:ソケットとXTI",
ピアソン・エデュケーション, 1999年. ISBN 4-98471-205-9
  3.9節 readn, writen, および readline 関数 (p.76)

Richard Stevens: "UNIX Network Programming, Volume 1, Second Edition:
Networking APIs: Sockets and XTI", Prentice Hall, 1998.
ISBN 0-13-490012-X.  
    Section 3.9 readn, writen, and readline Functions (p.77)

http://www.kohala.com/start/ (http://www.kohala.com/~rstevens/)
http://www.kohala.com/start/unpv12e/unpv12e.tar.gz

*/

/* include writen */
/*#include	"unp.h"*/
#include <errno.h>

ssize_t			/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

/* include readline */
/*#include	"unp.h"*/

#define	MAXLINE	4096

static ssize_t
my_read(int fd, char *ptr)
{
	static int	read_cnt = 0;
	static char	*read_ptr;
	static char	read_buf[MAXLINE];

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	int		n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			if (n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

