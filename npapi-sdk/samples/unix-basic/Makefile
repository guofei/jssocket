CFLAGS = -Wall -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -g

npbasicplugin:	BasicPlugin.o api.o js_scripting.o list.o ll.o
	cc $(CFLAGS) -shared BasicPlugin.o api.o js_scripting.o list.o ll.o -o npbasicplugin.so

BasicPlugin.o : BasicPlugin.c
	cc $(CFLAGS) -c BasicPlugin.c

api.o :	api.c
	cc $(CFLAGS) -c api.c

jsmethod.o : 	js_scripting.c
	cc $(CFLAGS) -c js_scripting.c

list.o:	list.c
	cc $(CFLAGS) -c list.c

ll.o:	ll.c
	cc $(CFLAGS) -c ll.c

BasicPlugin.o:	BasicPlugin.h
api.o:	api.h
jsmethod.o:	js_scripting.h
list.o:	list.h
ll.o:	ll.h


clean :
	rm npbasicplugin.so *.o

