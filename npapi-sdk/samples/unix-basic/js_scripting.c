#include "js_scripting.h"
#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LEN_OF_FUNCNAME 50

typedef enum __FUNCNAMES{
	TCP_CONNECT = 1,
	TCP_SEND,
	TCP_RECV,
	FN_SRPC_GETUSERLIST,
	FN_SRPC_ISEXISTUSER,
	FN_SRPC_ISONLINEUSER,
	FN_SRPC_CONNECT,
	FN_SRPC_DISCONNECT,
	FN_SRPC_CALL,
	CLOSE,
	TEST,
	NUM_OF_FUNCS
}FUNCNAMES;

static char arrayFuncNames[NUM_OF_FUNCS][LEN_OF_FUNCNAME] = {
	{ "" },
	{ "tcp_connect" },
	{ "tcp_send" },
	{ "tcp_recv" },
	{ "srpc_getUserList" },
	{ "srpc_isExistUser" },
	{ "srpc_isOnlineUser" },
	{ "srpc_connect" },
	{ "srpc_disconnect" },
	{ "srpc_call" },
	{ "close" },
	{"test"}
};

//enum __FUNNAMESが返る
int chkMethod(char *target)
{
	int nRet = 0;
	int i;

	for( i=1; i<NUM_OF_FUNCS; i++ ){
		if( 0 == strcmp( target, arrayFuncNames[i] )){
			nRet = i;
			break; 
		}
	}
	return nRet;
}

bool hasMethod(NPObject *obj, NPIdentifier methodName){
	NPUTF8 *name = sBrowserFuncs->utf8fromidentifier( methodName );

	bool result = false;
	int nFuncType = 0;
  
	if( 0 < (nFuncType =  chkMethod( name )) ){
		result = true;
	}

	sBrowserFuncs->memfree( name );
	//  return 1;
	return result;
}

bool invoke(NPObject *obj, NPIdentifier methodName,const NPVariant *args,uint32_t argCount,NPVariant *result){
	NPUTF8 *name = sBrowserFuncs->utf8fromidentifier( methodName );
	int nType = chkMethod( name );
	
	DebugMsg("in invoke\n");

	int i;
	NPString str;
	static char buf[1024];

	switch( nType ){
	case TEST:
		DebugMsg("in test\n");
		{
		char* message = "Hello from C";
		DebugMsg(message);

		// Get window object.
		NPObject* window = NULL;
		sBrowserFuncs->getvalue(mynpp->npp, NPNVWindowNPObject, &window);

		// Get console object.
		NPVariant consoleVar;
		NPIdentifier id = sBrowserFuncs->getstringidentifier("console");
		sBrowserFuncs->getproperty(mynpp->npp, window, id, &consoleVar);
		NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

		// Get the debug object.
		id = sBrowserFuncs->getstringidentifier("debug");

		// Invoke the call with the message!
		NPVariant type;
		STRINGZ_TO_NPVARIANT(message, type);
		NPVariant args[] = { type };
		NPVariant voidResponse;
		sBrowserFuncs->invoke(mynpp->npp, console, id, args,
			   sizeof(args) / sizeof(args[0]),
			   &voidResponse);

		// Cleanup all allocated objects, otherwise, reference count and
		// memory leaks will happen.
		sBrowserFuncs->releaseobject(window);
		sBrowserFuncs->releasevariantvalue(&consoleVar);
		sBrowserFuncs->releasevariantvalue(&voidResponse);
		}
		INT32_TO_NPVARIANT(1,*result);
		break;
	case TCP_CONNECT:
		sBrowserFuncs->memfree( name );
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 2){
			str = NPVARIANT_TO_STRING( args[0] );
			i = NPVARIANT_TO_INT32( args[1] );
			DebugMsg((char *)str.UTF8Characters);

			int sock = tcp_connect((char *)str.UTF8Characters, i );

			INT32_TO_NPVARIANT(sock, *result);
			return true;    
		}

		break;
	case TCP_SEND:
		sBrowserFuncs->memfree( name );
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 2){
			str = NPVARIANT_TO_STRING( args[1] );//msg 
			i = NPVARIANT_TO_INT32( args[0] );//socket
			DebugMsg((char *)str.UTF8Characters);
			
			int slen = writen(i,(char *)str.UTF8Characters, strlen((char *)str.UTF8Characters));

			INT32_TO_NPVARIANT(slen, *result);
			return true;    
		}
		break;
	case TCP_RECV:
		sBrowserFuncs->memfree( name );
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 1){
			i = NPVARIANT_TO_INT32( args[0] );
		
			int slen = readline(i,buf,1024 );

			//STRING_TO_NPVARIANT(buf, *result);
			DebugMsg(buf);
			return true;    
		}
		break;
	case CLOSE:
		sBrowserFuncs->memfree( name );
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 1){
			i = NPVARIANT_TO_INT32( args[0] );
			close(i);
			return true;    
		}
		break;
	default:
		break;
	}

	sBrowserFuncs->memfree( name );

	return false;
}
