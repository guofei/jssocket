#include "js_scripting.h"
#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LEN_OF_FUNCNAME 50

typedef enum __FUNCNAMES{
	CONNECT = 1,
	FN_KVS_IN,
	FN_KVS_RD,
	FN_SRPC_GETUSERLIST,
	FN_SRPC_ISEXISTUSER,
	FN_SRPC_ISONLINEUSER,
	FN_SRPC_CONNECT,
	FN_SRPC_DISCONNECT,
	FN_SRPC_CALL,
	FN_TEST_GETHW,
	NUM_OF_FUNCS
}FUNCNAMES;

static char arrayFuncNames[NUM_OF_FUNCS][LEN_OF_FUNCNAME] = {
	{ "" },
	{ "connect" },
	{ "kvs_in" },
	{ "kvs_rd" },
	{ "srpc_getUserList" },
	{ "srpc_isExistUser" },
	{ "srpc_isOnlineUser" },
	{ "srpc_connect" },
	{ "srpc_disconnect" },
	{ "srpc_call" },
	{ "GetHw" }
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

	switch( nType ){
	case CONNECT:
		sBrowserFuncs->memfree( name );
		BOOLEAN_TO_NPVARIANT( false, *result);
		DebugMsg("in CONNECT\n");
		if(argCount == 2 && NPVARIANT_IS_STRING(args[0]) && NPVARIANT_IS_INT32(args[1])){
		  DebugMsg("in if\n");
			str = NPVARIANT_TO_STRING( args[0] );
			i = NPVARIANT_TO_INT32( args[1] );
			DebugMsg((char *)str.UTF8Characters);
			/*
			  do connect
			*/
			int sock = tcp_connect((char *)str.UTF8Characters, i );

			INT32_TO_NPVARIANT(sock, *result);
			return true;    
		}

		break;
	default:
		break;
	}

	sBrowserFuncs->memfree( name );

	return false;
}
