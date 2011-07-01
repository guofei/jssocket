#include "api.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//debug message print to file
void DebugMsg( char *msg )
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
