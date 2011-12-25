#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "js_scripting.h"
#include "api.h"
#include "list.h"

#define LEN_OF_FUNCNAME 50
pthread_mutex_t mutex;

static char arrayFuncNames[NUM_OF_FUNCS][LEN_OF_FUNCNAME] = {
	{ "" },
	{ "tcp_connect" },
	{ "tcp_send" },
	{ "tcp_recv" },
	{ "tcp_acc_port" },
	{ "tcp_accept" },
	{ "close" },
	{ "test" }
};

struct socket_event
{
	FUNCNAMES name;
	int s;
	void *buf;
	NPObject *obj;
	int n;
};

static void callback_to_javascript(void *func_args);

void callback_to_javascript(void *func_args)
{
	//async_args *args = (async_args*)func_args;
	//NPObject *args = (NPObject *)func_args;
	async_args *tmp = (async_args *)func_args;
	if( tmp->n == TCP_CONNECT ){
		NPVariant type;
		int s = tmp->s;
		INT32_TO_NPVARIANT(s,type);
		NPVariant myargs[] = { type };
		NPVariant voidResponse;
		sBrowserFuncs->invokeDefault( plugin_instance_data->npp, tmp->obj, myargs, 1, &voidResponse );
		//sBrowserFuncs->releaseobject( args->func );
		free(tmp);
		sBrowserFuncs->releasevariantvalue(&voidResponse);

	}
	else if( tmp->n == TCP_RECV ){
		NPVariant type;

		//FIXME
		STRINGZ_TO_NPVARIANT(tmp->buf,type);
		DebugMsg("tmp->buf:");
		DebugMsg(tmp->buf);
		DebugMsg("\n");
		NPVariant myargs[] = { type };
		NPVariant voidResponse;
		DebugMsg("in test asy!!!!!!!!!!!!!!!!!!!!!!!tcp recv\n");
		sBrowserFuncs->invokeDefault( plugin_instance_data->npp, tmp->obj, myargs, 1, &voidResponse );
		DebugMsg("in test asy!!!!!!!!!!!!!!!!!!!!!!!tcp recv\n");
		//sBrowserFuncs->releaseobject( args->func );
		free(tmp);
		sBrowserFuncs->releasevariantvalue(&voidResponse);
	}
}

void* threadfunc(void* p)
{
	pthread_detach(pthread_self());
	NPP instance = (NPP)p;
	InstanceData *tmp_ins = instance->pdata;
	list_t *list = &tmp_ins->list;
	while(1){
		int s;
		struct socket_event *event = list_pop(list);
		if( event != NULL){
			if (event->name == TCP_CONNECT){
				char *server = (char *)event->buf;
				s = tcp_connect(server,event->s);
				async_args *args = malloc(sizeof(async_args));
				args->obj = event->obj;
				args->s = s;
				args->n = TCP_CONNECT;
				sBrowserFuncs->pluginthreadasynccall( instance, callback_to_javascript, args );
			}
			else if(event->name == TCP_SEND){
				char *m = (char *)event->buf;
				DebugMsg(m);

				writen(event->s,m,strlen(m));
			}
			else if(event->name == TCP_RECV){
				char buf[1024];
				readline(event->s, buf, 1024);
				DebugMsg("recv buf:");
				DebugMsg(buf);
				async_args *args = malloc(sizeof(async_args));
				args->obj = event->obj;
				args->n = TCP_RECV;
				strncpy(args->buf,buf,strlen(buf));
				sBrowserFuncs->pluginthreadasynccall( instance, callback_to_javascript, args );

			}
			else if(event->name == TCP_ACC_PORT){

			}
			else if(event->name == TCP_ACCEPT){

			}
			else if(event->name == CLOSE){
				DebugMsg("close");
				close(event->s);
			}

			free(event);
		}
	}
	return NULL;
}

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
	sBrowserFuncs->memfree( name );

	list_t *list = &plugin_instance_data->list;
	int i,port;
	char *buf;
	NPString str;
	NPObject *callback_obj;
	struct socket_event *event = malloc(sizeof(struct socket_event));

	switch(nType){
	case TCP_CONNECT:
		if(argCount != 3 || !NPVARIANT_IS_STRING(args[0]) || (!NPVARIANT_IS_INT32(args[1]) && !NPVARIANT_IS_DOUBLE(args[1]))
		   || !NPVARIANT_IS_OBJECT(args[2]))
			goto error;

		str = NPVARIANT_TO_STRING( args[0] );

		if(NPVARIANT_IS_INT32(args[1]))
			port = NPVARIANT_TO_INT32( args[1] );
		else if(NPVARIANT_IS_DOUBLE(args[1]))
			port = NPVARIANT_TO_DOUBLE( args[1] );

		callback_obj = NPVARIANT_TO_OBJECT( args[2] );
		sBrowserFuncs->retainobject( callback_obj );

		event->name = TCP_CONNECT;
		event->buf = strndup((char *)str.UTF8Characters, str.UTF8Length+1);
		buf = event->buf;
		buf[str.UTF8Length] = '\0';
		event->s = port;
		event->obj = callback_obj;
		list_push(list,event);

		INT32_TO_NPVARIANT(0, *result);
		return true;
	case TCP_SEND:
		if(argCount != 2 || (!NPVARIANT_IS_INT32(args[0]) && !NPVARIANT_IS_DOUBLE(args[0])) || !NPVARIANT_IS_STRING(args[1]))
			goto error;

		//socket
		if(NPVARIANT_IS_INT32(args[0]))
			i = NPVARIANT_TO_INT32( args[0] );
		else if(NPVARIANT_IS_DOUBLE(args[0]))
			i = NPVARIANT_TO_DOUBLE( args[0] );

		str = NPVARIANT_TO_STRING(args[1]); //msg
		event->buf = malloc(str.UTF8Length+2);
		event->buf = strncpy(event->buf, (char *)str.UTF8Characters, str.UTF8Length);
		buf = event->buf;
		buf[str.UTF8Length] = '\0';
		buf[str.UTF8Length+1] = '\0';
		//sprintf(buf, "%s\n",buf);

		event->name = TCP_SEND;
		event->s = i;

		list_push(list,event);
		INT32_TO_NPVARIANT(1, *result);
		return true;
	case TCP_RECV:
		if(argCount != 2 || (!NPVARIANT_IS_INT32(args[0]) && !NPVARIANT_IS_DOUBLE(args[0])))
			goto error;

		//socket
		if(NPVARIANT_IS_INT32(args[0]))
			i = NPVARIANT_TO_INT32( args[0] );
		else if(NPVARIANT_IS_DOUBLE(args[0]))
			i = NPVARIANT_TO_DOUBLE( args[0] );

		callback_obj = NPVARIANT_TO_OBJECT(args[1]);
		sBrowserFuncs->retainobject( callback_obj );

		event->name = TCP_RECV;
		event->s = i;
		event->obj = callback_obj;
		list_push(list,event);
		return true;
	case CLOSE:
		if(argCount != 1 || (!NPVARIANT_IS_INT32(args[0]) && !NPVARIANT_IS_DOUBLE(args[0])))
			goto error;

		//socket
		if(NPVARIANT_IS_INT32(args[0]))
			i = NPVARIANT_TO_INT32( args[0] );
		else if(NPVARIANT_IS_DOUBLE(args[0]))
			i = NPVARIANT_TO_DOUBLE( args[0] );

		event->name = CLOSE;
		event->s = i;
		list_push(list,event);
		return true;
	default:
		return false;
	}
error:
	INT32_TO_NPVARIANT(-1, *result);
	return false;
}
