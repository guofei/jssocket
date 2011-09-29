#ifndef js_scripting_h_
#define js_scripting_h_

#include "BasicPlugin.h"
extern bool hasMethod(NPObject *obj, NPIdentifier methodName);
extern bool invoke(NPObject *obj, NPIdentifier methodName,const NPVariant *args,uint32_t argCount,NPVariant *result);
extern void* threadfunc_do_socket_event(void* p);
extern pthread_mutex_t mutex;

#endif
