#include "api.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>  /* socket() */
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <netdb.h>      /* getaddrinfo() */


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
		pid = getpid();
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
  if( (err = getaddrinfo( server, portno_str, &hints, &ai )) )
    {
      fprintf(stderr,"unknown server %s (%s)\n",server,
	      gai_strerror(err) );
      goto error0;
    }
  if( (s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 )
    {
      perror("socket");
      goto error1;
    }
  if( connect(s, ai->ai_addr, ai->ai_addrlen) < 0 )
    {
      perror( server );
      goto error2;
    }
  char *Text = "This is a test";
  send(s, Text, strlen(Text)+1, 0);
  freeaddrinfo( ai );
  
  return( s );
 error2:
  close( s );
 error1:
  freeaddrinfo( ai );
 error0:
  return( -1 );
}
