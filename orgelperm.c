#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/io.h>
#include<sys/capability.h>
#include<sys/prctl.h>
#include<error.h>
#include"orgel-io.h"

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))


int main(int argc, char *argv[]){
  int i;
  cap_t caps;
  cap_value_t rawio_cap[]={CAP_SYS_RAWIO},
              cap_list[]={CAP_IPC_LOCK, CAP_SYS_NICE};

  if(!argv[1]){
    fprintf(stderr, "Usage: %s <command>\n", argv[0]);
    exit(1);
    }

  caps=cap_get_proc();

  // First enable CAP_SYS_RAWIO, which is needed only for the ioperm() call
  // below and should not be included in the set of capabilities that are
  // passed on across the exec. Passing it on could be a security hole,
  // since it would allow the execed program to enable access to arbitrary
  // hardware I/O ports besides those needed for the organ.

  if(cap_set_flag(caps, CAP_EFFECTIVE, 1, rawio_cap, CAP_SET))
    error(1, errno, "cap_set_flag()");
  if(cap_set_proc(caps))
    error(1, errno, "cap_set_proc()");

  // Enable I/O port access to the organ.

  ioperm(ORGEL_IO_PORTS_BASE, ORGEL_IO_PORTS_LEN, 1);

  // Now enable the other capabilities that are needed by the organ software,
  // and which should be passed on to the execed program. This means they
  // have to be effective, inheritable, and ambient; the latter is handled
  // with prctl() below.

  if(cap_set_flag(caps, CAP_EFFECTIVE, ARRLEN(cap_list), cap_list, CAP_SET))
    error(1, errno, "cap_set_flag()");
  if(cap_set_flag(caps, CAP_INHERITABLE, ARRLEN(cap_list), cap_list, CAP_SET))
    error(1, errno, "cap_set_flag()");
  if(cap_set_proc(caps))
    error(1, errno, "cap_set_proc()");

  cap_free(caps);

  for(i=0; i<ARRLEN(cap_list); ++i)
    prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap_list[i], 0, 0);

  ++argv;
  execv(argv[0], argv);

  error(1, errno, "Cannot exec %s", argv[0]);
  }
