#include "js_scripting.h"
#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

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

typedef struct _socket_event
{
	FUNCNAMES name;
	int s;
	void *buf;
	NPObject *obj;
	int n;
	struct _socket_event *next;
}socket_event;

socket_event *event_queue;

static int add_event(socket_event *event);
static int del_event(socket_event *fist);

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
	while(1){
		int s;
		if(event_queue != NULL){
			if (event_queue->name == TCP_CONNECT){
				char *server = (char *)event_queue->buf;
				s = tcp_connect(server,event_queue->s);
				async_args *args = malloc(sizeof(async_args));
				args->obj = event_queue->obj;
				args->s = s;
				args->n = TCP_CONNECT;
				sBrowserFuncs->pluginthreadasynccall( plugin_instance_data->npp, callback_to_javascript, args );
			}
			else if(event_queue->name == TCP_SEND){
				char *m = (char *)event_queue->buf;
				DebugMsg(m);

				writen(event_queue->s,m,strlen(m));
			}
			else if(event_queue->name == TCP_RECV){
				char buf[1024];
				readline(event_queue->s, buf, 1024);
				DebugMsg("recv buf:");
				DebugMsg(buf);
				async_args *args = malloc(sizeof(async_args));
				args->obj = event_queue->obj;
				args->n = TCP_RECV;
				strncpy(args->buf,buf,strlen(buf));
				//args->buf[0] = 'h';args->buf[1] = 'i'; args->buf[2] = '\0';
				sBrowserFuncs->pluginthreadasynccall( plugin_instance_data->npp, callback_to_javascript, args );

			}
			else if(event_queue->name == TCP_ACC_PORT){

			}
			else if(event_queue->name == TCP_ACCEPT){

			}
			else if(event_queue->name == CLOSE){
				DebugMsg("close");
				close(event_queue->s);
			}

			del_event(event_queue);
		}
	}
	
	return NULL;  
}


void test_asy(void *func_args)
{
	//async_args *args = (async_args*)func_args;
	NPObject *args = (NPObject *)func_args;
	
	NPVariant type;
	STRINGZ_TO_NPVARIANT("12345",type);
	NPVariant myargs[] = { type };
	NPVariant voidResponse;
	sBrowserFuncs->invokeDefault( plugin_instance_data->npp, args, myargs, 1, &voidResponse );
	DebugMsg("in test asy!!!!!!!!!!!!!!!!!!!!!!!\n");
	//sBrowserFuncs->releaseobject( args->func );
	sBrowserFuncs->releasevariantvalue(&voidResponse);
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

	DebugMsg("in invoke\n");
	
	int i;
	NPString str;
	static char buf[1024];
	socket_event *event = malloc(sizeof(socket_event));

	switch( nType ){
	case TEST:
		DebugMsg("in test\n");
		{
		char* message = "Hello from C";
		DebugMsg(message);

		// Get window object.
		NPObject* window = NULL;
		sBrowserFuncs->getvalue(plugin_instance_data->npp, NPNVWindowNPObject, &window);

		// Get console object.
		NPVariant consoleVar;
		NPIdentifier id = sBrowserFuncs->getstringidentifier("console");
		sBrowserFuncs->getproperty(plugin_instance_data->npp, window, id, &consoleVar);
		NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

		// Get the debug object.
		id = sBrowserFuncs->getstringidentifier("debug");

		// Invoke the call with the message!
		NPVariant type;
		STRINGZ_TO_NPVARIANT(message, type);
		NPVariant myargs[] = { type };
		NPVariant voidResponse;
		sBrowserFuncs->invoke(plugin_instance_data->npp, console, id, myargs,
			   sizeof(myargs) / sizeof(myargs[0]),
			   &voidResponse);

		// Cleanup all allocated objects, otherwise, reference count and
		// memory leaks will happen.
		sBrowserFuncs->releaseobject(window);
		sBrowserFuncs->releasevariantvalue(&consoleVar);
		sBrowserFuncs->releasevariantvalue(&voidResponse);
		}
		INT32_TO_NPVARIANT(1,*result);
		return true;
		break;
	case TCP_CONNECT:
		BOOLEAN_TO_NPVARIANT( false, *result);
		int ret = 0,s;
		if(argCount == 3){
			if( NPVARIANT_IS_STRING( args[0] ) )
				str = NPVARIANT_TO_STRING( args[0] );
			else{
				INT32_TO_NPVARIANT(-1, *result);
				return true;
			}
			
			if(NPVARIANT_IS_INT32(args[1]))
				s = NPVARIANT_TO_INT32( args[1] );
			else if(NPVARIANT_IS_DOUBLE(args[1]))
				s = NPVARIANT_TO_DOUBLE( args[1] );
			else{
				INT32_TO_NPVARIANT(-1, *result);
				return true;
			}
			
			NPObject *callback_obj;
			if( NPVARIANT_IS_OBJECT(args[2]) )
				callback_obj = NPVARIANT_TO_OBJECT( args[2] );
			else{
				INT32_TO_NPVARIANT(-1, *result);
				return true;
			}
			sBrowserFuncs->retainobject( callback_obj );

			event->name = TCP_CONNECT;
			event->buf = strndup((char *)str.UTF8Characters, str.UTF8Length+1);
			char *buf = event->buf;
			buf[str.UTF8Length] = '\0';
			event->s = s;
			event->obj = callback_obj;
			event->next = NULL;
			add_event(event);
		}
		INT32_TO_NPVARIANT(ret, *result);
		return true;    
		break;
	case TCP_SEND:
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 2){
			if( NPVARIANT_IS_STRING( args[1] ) )
				str = NPVARIANT_TO_STRING( args[1] ); //msg
			else{
				INT32_TO_NPVARIANT(-1, *result);
				return true;
			}

			strncpy(buf,(char *)str.UTF8Characters,strlen((char *)str.UTF8Characters));

			/* event->buf = strndup((char *)str.UTF8Characters,str.UTF8Length+1); */
			/* char *buf = event->buf; */
			buf[str.UTF8Length] = '\0';
			sprintf(buf, "%s\n",buf);

			if(NPVARIANT_IS_INT32(args[0]))
				i = NPVARIANT_TO_INT32( args[0] );
			else if(NPVARIANT_IS_DOUBLE(args[0]))
				i = NPVARIANT_TO_DOUBLE( args[0] );

			//i = NPVARIANT_TO_INT32( args[0] );//socket
			//DebugMsg((char *)str.UTF8Characters);

			event->name = TCP_SEND;
			event->s = i;
			char strr[10];
			sprintf(strr, "%d", event->s);
			DebugMsg("tcp send:");
			DebugMsg(strr);

			event->buf = malloc(sizeof(char)*(strlen(buf)+1));
			event->buf = strncpy(event->buf,buf,strlen(buf)+1);
			event->next = NULL;
			add_event(event);
			//int slen = writen(i,(char *)str.UTF8Characters, strlen((char *)str.UTF8Characters));

			INT32_TO_NPVARIANT(1, *result);
			//DebugMsg("send ok\n");
			return true;    
		}
		break;
	case TCP_RECV:
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 2){
			if(NPVARIANT_IS_INT32(args[0]))
				i = NPVARIANT_TO_INT32( args[0] );
			else if(NPVARIANT_IS_DOUBLE(args[0]))
				i = NPVARIANT_TO_DOUBLE( args[0] );

			//i = NPVARIANT_TO_INT32( args[0] );
			
			NPObject *callback_obj;
			callback_obj = NPVARIANT_TO_OBJECT( args[1] );
			sBrowserFuncs->retainobject( callback_obj );

			event->name = TCP_RECV;
			event->s = i;
			event->obj = callback_obj;
			add_event(event);
			//async_args myargs;
			//myargs.func = callback_obj;
			//int slen = readline( i,buf,1024 );
			//DebugMsg(buf);
			//sBrowserFuncs->pluginthreadasynccall( plugin_instance_data->npp, test_asy, callback_obj );
			//DebugMsg("in tcp recv\n");			
			//STRING_TO_NPVARIANT(buf, *result);
			
			return true;    
		}
		break;
	case CLOSE:
		BOOLEAN_TO_NPVARIANT( false, *result);
		if(argCount == 1){
			if(NPVARIANT_IS_INT32(args[0]))
				i = NPVARIANT_TO_INT32( args[0] );
			else if(NPVARIANT_IS_DOUBLE(args[0]))
				i = NPVARIANT_TO_DOUBLE( args[0] );

			event->name = CLOSE;
			event->s = i;
			add_event(event);
			return true;    
		}
		break;
	default:
		break;
	}

	return false;
}

int add_event(socket_event *event)
{
	pthread_mutex_lock(&mutex);
	if( event == NULL ){
		return -1;
	}

	if( event_queue == NULL ){
		event_queue = event;
	}
	else{
		socket_event *tmp = event_queue;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = event;
	}

	pthread_mutex_unlock(&mutex);
	return 0;
}

int del_event(socket_event *fist)
{
	pthread_mutex_lock(&mutex);
	event_queue = fist->next;
	//free(fist->buf);
	free(fist);
	pthread_mutex_unlock(&mutex);
	return 0;
}

