# Basic plugin requires GTK and X11
CFLAGS = -Wall -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -g

npbasicplugin : BasicPlugin.o api.o jsmethod.o
	cc $(CFLAGS) -shared BasicPlugin.o api.o jsmethod.o -o npbasicplugin.so

BasicPlugin.o : BasicPlugin.c BasicPlugin.h
	cc `pkg-config --cflags gtk+-2.0` $(CFLAGS) -c BasicPlugin.c

api.o : api.c api.h
	cc `pkg-config --cflags gtk+-2.0` $(CFLAGS) -c api.c

jsmethod.o : jsmethod.c jsmethod.h
	cc `pkg-config --cflags gtk+-2.0` $(CFLAGS) -c jsmethod.c

clean :
	rm npbasicplugin.so BasicPlugin.o jsmethod.o

