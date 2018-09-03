CFLAGS := -O2 -std=c99 -Wall -fplan9-extensions

MODULES := mog prio na sid add slew lfo
MODOBJS := $(patsubst %,%.o,$(MODULES))

euler-demo: all_modules.c main.o modules.o jacks.o commands.o $(MODOBJS)
	gcc ${CFLAGS} -o euler-demo main.o modules.o jacks.o commands.o \
	    $(MODOBJS) \
	    -lpthread -lreadline -ltermcap -lm

.PHONY: all_modules.c
all_modules.c:
	echo extern class $(MODULES:%=%_class,) | sed 's/,$$/;/' \
	     > all_modules.c
	echo 'class *all_classes[]={' $(MODULES:%='&%_class,') ' 0};' \
	     >> all_modules.c

jackspec: jackspec.c
	gcc ${CFLAGS} -o jackspec jackspec.c

%.o: %.c orgel-io.h orgel.h
	gcc -c ${CFLAGS} -o $@ $<

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
