#include "api.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>  /* socket() */
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <netdb.h>      /* getaddrinfo() */
#include <errno.h>

#define PORTNO_BUFSIZE 30

//debug message print to file
void 
DebugMsg( char *msg )
{
	FILE *fp = NULL;
	static int nfStartup = 0;
	int pid = 0;

	if( NULL != (fp = fopen( "./debug-log.txt", "a+" )) ){

		if( 0 == nfStartup ){
			nfStartup = 1;
			fprintf( fp, "::first call");
		}
		pid = pthread_self();
		fprintf( fp, "::pid(%6d)::", pid );
		fputs( msg, fp );
		fclose( fp );
	}
}

int
tcp_connect(char *ser, int portno)
{
	DebugMsg("in tcp conncet\n");
	struct addrinfo hints, *ai;
	char *server = strdup(ser);
	char portno_str[PORTNO_BUFSIZE];
	int s, err;
	snprintf( portno_str,sizeof(portno_str),"%d",portno );
	memset( &hints, 0, sizeof(hints) );
	hints.ai_socktype = SOCK_STREAM;
	if( (err = getaddrinfo( server, portno_str, &hints, &ai )) ){
		fprintf(stderr,"unknown server %s (%s)\n",server,
			gai_strerror(err) );
		goto error0;
	}
	if( (s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 ){
		perror("socket");
		goto error1;
	}
	if( connect(s, ai->ai_addr, ai->ai_addrlen) < 0 ){
		perror( server );
		goto error2;
	}

	freeaddrinfo( ai );
  
	return( s );
error2:
	close( s );
error1:
	freeaddrinfo( ai );
error0:
	return( -1 );
}

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

ssize_t 
readn(int fd, void *vptr, size_t n)
{
	ssize_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while( nleft > 0 ){
		if( ( nread = read(fd,ptr,nleft) ) < 0 ){
			if( errno == EINTR )
				nread = 0;
			else
				return -1;
		}
		else if( nread == 0 )
			break;
		nleft -= nread;
		ptr += nread;
	}
	return n-nleft;
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	DebugMsg("in readline\n");
	ssize_t	n, rc;
	char	c, *ptr;
	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
again:
		if ( (rc = read(fd, &c, 1)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			if (n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		} else {
			if (errno == EINTR)
				goto again;
			return(-1);		/* error, errno set by read() */
		}
	}
	DebugMsg(vptr);
	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

int
tcp_acc_port( int portno, int ip_version )
{
	struct addrinfo hints, *ai;
	char portno_str[PORTNO_BUFSIZE];
	int err, s, on, pf;

    	switch( ip_version )
	{
	case 4:
		pf = PF_INET;
		break;
	case 6:
		pf = PF_INET6;
		break;
	default:
		fprintf(stderr,"bad IP version: %d.  4 or 6 is allowed.\n",
			ip_version );
		goto error0;
	}
	snprintf( portno_str,sizeof(portno_str),"%d",portno );
	memset( &hints, 0, sizeof(hints) );
	ai = NULL;
	hints.ai_family   = pf ;
	hints.ai_flags    = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM ;
	if( (err = getaddrinfo( NULL, portno_str, &hints, &ai )) )
	{
		fprintf(stderr,"bad portno %d? (%s)\n",portno,
			gai_strerror(err) );
		goto error0;
	}
	if( (s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 )
	{
		perror("socket");
		goto error1;
	}

#ifdef	IPV6_V6ONLY
	if( ai->ai_family == PF_INET6 )
	{
		on = 1;
		if( setsockopt(s,IPPROTO_IPV6, IPV6_V6ONLY,&on,sizeof(on)) < 0 )
		{
			perror("setsockopt(,,IPV6_V6ONLY)");
			goto error1;
		}
	}
#endif	/*IPV6_V6ONLY*/

	if( bind(s,ai->ai_addr,ai->ai_addrlen) < 0 )
	{
		perror("bind");
		fprintf(stderr,"port number %d can be already used. wait a moment or kill another program.\n", portno );
		goto error2;
	}
	on = 1;
	if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) < 0 )
	{
		perror("setsockopt(,,SO_REUSEADDR)");
		goto error2;
	}
	if( listen( s, 5 ) < 0 )
	{
		perror("listen");
		goto error2;
	}
	freeaddrinfo( ai );
	return( s );

error2:
	close( s );	
error1:
	freeaddrinfo( ai );
error0:
	return( -1 );
}

