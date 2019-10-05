CFLAGS := -g -O2 -std=c99 -Wall
LDFLAGS := -lpthread -lreadline -ltermcap -lm -lmcheck -ldl -rdynamic

MODULES := sid mog prio keyboard na add slew lfo gdelay sdelay adsr cquencer \
           crums sah random conv poly pulse
# pulse
MODOBJS := $(patsubst %,%.o,$(MODULES))

hardware := yes

ifeq ($(hardware),no)
  CFLAGS += -DNO_HARDWARE -Wno-unused-value
endif

gui :=

GUI_DIR := $(if $(gui),$(gui)-gui)
GUI_OBJ := $(if $(gui),$(GUI_DIR)/gui.o)

all: euler-demo

euler-demo: all_modules.c main.o modules.o jacks.o commands.o cmdlex.o \
	    util.o $(MODOBJS) $(GUI_OBJ)
	gcc ${CFLAGS} -o euler-demo main.o modules.o jacks.o commands.o \
	    cmdlex.o util.o $(MODOBJS) $(GUI_OBJ) $(LDFLAGS)

testing: all_modules.c modinit.o modules.o jacks.o commands.o cmdlex.o \
	 util.o $(MODOBJS)
	gcc ${CFLAGS} -o testing modinit.o modules.o jacks.o commands.o \
	cmdlex.o util.o $(MODOBJS) $(LDFLAGS)

.PHONY: $(GUI_OBJ)
$(GUI_OBJ):
	$(MAKE) -C $(GUI_DIR)

.PHONY: all_modules.c
all_modules.c:
	echo extern class $(MODULES:%=%_class,) | sed 's/,$$/;/' \
	     > all_modules.c
	echo 'class *all_classes[]={' $(MODULES:%='&%_class,') ' 0};' \
	     >> all_modules.c

jackspec: jackspec.c
	gcc ${CFLAGS} -o jackspec jackspec.c

%.o: %.c orgel-io.h orgel.h
	gcc -c ${CFLAGS} $(if $(findstring $@,$(MODOBJS)),-DCOMPILING_MODULE) \
	    -o $@ $<

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
	rm -f *.o orgelperm euler-demo jackspec *.spec.c
	$(if $(gui),$(MAKE) -C $(GUI_DIR) clean)
