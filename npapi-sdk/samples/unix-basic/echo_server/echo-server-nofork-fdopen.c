
/*
	echo-server-nofork-fdopen.c -- 受け取った文字列をそのまま返すサーバ(fork無し版)
	~yas/syspro/ipc/echo-server-nofork-fdopen.c
	Created on 2004/05/09 19:08:47
*/
#include <stdio.h>
#include <stdlib.h>	/* exit() */
#include <sys/types.h>	/* socket(), wait4() */
#include <sys/socket.h>	/* socket() */
#include <netinet/in.h>	/* struct sockaddr_in */
#include <sys/resource.h> /* wait4() */
#include <sys/wait.h>	/* wait4() */
#include <netdb.h>	/* getnameinfo() */
#include <string.h>	/* strlen() */

extern	void echo_server( int portno, int ip_version );
extern	void echo_receive_request_and_send_reply( int com );
extern	void print_my_host_port( int portno );
extern	void tcp_peeraddr_print( int com );
extern	void sockaddr_print( struct sockaddr *addrp, socklen_t addr_len );
extern	int  tcp_acc_port( int portno, int pf );
extern	int fdopen_sock( int sock, FILE **inp, FILE **outp );

main( int argc, char *argv[] )
{
    int portno, ip_version;
        if( !(argc == 2 || argc==3) ) {
	    fprintf(stderr,"Usage: %s portno {ipversion}\n",argv[0] );
	    exit( 1 );
	}
	portno = strtol( argv[1],0,10 );
	if( argc == 3 )
	    ip_version = strtol( argv[2],0,10 );
	else
	    ip_version = 4; /* IPv4 by default */
	echo_server( portno, ip_version );
}

void
echo_server( int portno, int ip_version )
{
    int acc,com ;
	acc = tcp_acc_port( portno, ip_version );
	if( acc<0 )
	    exit( -1 );
	print_my_host_port( portno );
	while( 1 )
	{
	    if( (com = accept( acc,0,0 )) < 0 )
	    {
		perror("accept");
		exit( -1 );
	    }
	    tcp_peeraddr_print( com );
	    echo_receive_request_and_send_reply( com );
	}
}

#define	BUFFERSIZE	1024

void
echo_receive_request_and_send_reply( int com )
{
    char line[BUFFERSIZE] ;
    int rcount ;
    int wcount ;
    FILE *in, *out ;

	if( fdopen_sock(com,&in,&out) < 0 )
	{
	    fprintf(stderr,"fdooen()\n");
	    exit( 1 );
	}
	while( fgets(line,BUFFERSIZE,in) )
	{
	    rcount = strlen( line );
	    printf("[%d] received (fd==%d) %d bytes, [%s]\n",getpid(),com,rcount,line );
	    fflush( stdout );
	    fprintf(out,"%s",line );
	}
	printf("[%d] connection (fd==%d) closed.\n",getpid(),com );
	fclose( in );
	fclose( out );
}

void
print_my_host_port( int portno )
{
    char hostname[100] ;
	gethostname( hostname,sizeof(hostname) );
	hostname[99] = 0 ;
	printf("run telnet %s(v6) %d \n",hostname, portno );
}

void
tcp_peeraddr_print( int com )
{
    struct sockaddr_storage addr ;
    socklen_t addr_len ; /* MacOSX: __uint32_t */
	addr_len = sizeof( addr );
    	if( getpeername( com, (struct sockaddr *)&addr, &addr_len  )<0 )
	{
	    perror("tcp_peeraddr_print");
	    return;
	}
    	printf("[%d] connection (fd==%d) from ",getpid(),com );
	sockaddr_print( (struct sockaddr *)&addr, addr_len );
	printf("\n");
}

void
sockaddr_print( struct sockaddr *addrp, socklen_t addr_len )
{
    char host[BUFFERSIZE] ;
    char port[BUFFERSIZE] ;
	if( getnameinfo(addrp, addr_len, host, sizeof(host),
			port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV)<0 )
	    return;
	if( addrp->sa_family == PF_INET )
    	    printf("%s:%s", host, port );
	else
    	    printf("[%s]:%s", host, port );
}

#define PORTNO_BUFSIZE 30

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

int
fdopen_sock( int sock, FILE **inp, FILE **outp )
{
    int sock2 ;
	if( (sock2=dup(sock)) < 0 )
	{
	    return( -1 );
	}
	if( (*inp = fdopen( sock2, "r" )) == NULL )
	{
	    close( sock2 );
	    return( -1 );
	}
	if( (*outp = fdopen( sock, "w" )) == NULL )
	{
	    fclose( *inp );
	    *inp = 0 ;
	    return( -1 );
	}
	setvbuf(*outp, (char *)NULL, _IONBF, 0);
	return( 0 );
}
