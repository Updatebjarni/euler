CFLAGS := -O2 -std=c99 -Wall

MODULES := mog
MODOBJS := $(patsubst %,%.o,$(MODULES))

euler-demo: main.o modules.o commands.o $(MODOBJS)
	gcc ${CFLAGS} -o euler-demo main.o modules.o commands.o $(MODOBJS) \
	    -lpthread -lreadline -ltermcap

jackspec: jackspec.c
	gcc ${CFLAGS} -o jackspec jackspec.c

%.c: orgel-io.h orgel.h

$(MODOBJS): %.o: %.spec.c

%.spec:  # Without this, GNU Make thinks .spec files depend on .spec.o files,
         # which depend on .spec.c files, which produces a circular dependency.

%.spec.c: %.spec jackspec
	./jackspec $< $@

orgelperm: orgelperm.o
	gcc ${CFLAGS} -o orgelperm orgelperm.o -lcap

install-orgelperm: orgelperm
	sudo sh -c \
	     "cp orgelperm /usr/bin/orgelperm && \
	      chown root.root /usr/bin/orgelperm && \
	      chmod 755 /usr/bin/orgelperm && \
	      /sbin/setcap cap_ipc_lock,cap_sys_nice,cap_sys_rawio+p \
	                   /usr/bin/orgelperm"

clean:
	rm -f *.o orgelperm euler-demo
