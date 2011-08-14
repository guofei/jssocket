#ifndef js_scripting_h_
#define js_scripting_h_

#include "BasicPlugin.h"
extern bool hasMethod(NPObject *obj, NPIdentifier methodName);
extern bool invoke(NPObject *obj, NPIdentifier methodName,const NPVariant *args,uint32_t argCount,NPVariant *result);

#endif
