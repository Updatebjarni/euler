CFLAGS := -O2 -std=c99 -Wall

euler-demo: main.o mog.o modules.o commands.o
	gcc -o euler-demo main.o mog.o modules.o commands.o \
	    -lpthread -lreadline -ltermcap

%.c: orgel-io.h orgel.h

orgelperm: orgelperm.o
	gcc -o orgelperm orgelperm.o -lcap

install-orgelperm: orgelperm
	sudo sh -c \
	     "cp orgelperm /usr/bin/orgelperm && \
	      chown root.root /usr/bin/orgelperm && \
	      chmod 755 /usr/bin/orgelperm && \
	      /sbin/setcap cap_ipc_lock,cap_sys_nice,cap_sys_rawio+p \
	                   /usr/bin/orgelperm"

clean:
	rm -f *.o orgelperm euler-demo
