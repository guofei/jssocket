#include "js_scripting.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool hasMethod(NPObject *obj, NPIdentifier methodName){
	DebugMsg("in hasMethod\n");
	return true;
}

bool invoke(NPObject *obj, NPIdentifier methodName,const NPVariant *args,uint32_t argCount,NPVariant *result){
	STRINGZ_TO_NPVARIANT(strdup("from plugin!"), *result);
	DebugMsg("in invoke\n");
	return true;    
}
