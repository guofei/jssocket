#ifndef js_scripting_h_
#define js_scripting_h_

#include "BasicPlugin.h"
extern bool hasMethod(NPObject *obj, NPIdentifier methodName);
extern bool invoke(NPObject *obj, NPIdentifier methodName,const NPVariant *args,uint32_t argCount,NPVariant *result);
extern void *threadfunc(void* instance);
extern pthread_mutex_t mutex;

typedef struct async_args{
	NPObject *obj;
	char buf[1024];
	int s;
	int n;
}async_args;

typedef enum __FUNCNAMES{
	TCP_CONNECT = 1,
	TCP_SEND,
	TCP_RECV,
	TCP_ACC_PORT,
	TCP_ACCEPT,
	CLOSE,
	TEST,
	NUM_OF_FUNCS
}FUNCNAMES;

#endif
