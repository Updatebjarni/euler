#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/io.h>
#include<sys/capability.h>
#include<sys/prctl.h>
#include"orgel-io.h"
#include"caps.h"

const char *cap_name[CAP_LAST_CAP+1] = {
  "cap_chown",
  "cap_dac_override",
  "cap_dac_read_search",
  "cap_fowner",
  "cap_fsetid",
  "cap_kill",
  "cap_setgid",
  "cap_setuid",
  "cap_setpcap",
  "cap_linux_immutable",
  "cap_net_bind_service",
  "cap_net_broadcast",
  "cap_net_admin",
  "cap_net_raw",
  "cap_ipc_lock",
  "cap_ipc_owner",
  "cap_sys_module",
  "cap_sys_rawio",
  "cap_sys_chroot",
  "cap_sys_ptrace",
  "cap_sys_pacct",
  "cap_sys_admin",
  "cap_sys_boot",
  "cap_sys_nice",
  "cap_sys_resource",
  "cap_sys_time",
  "cap_sys_tty_config",
  "cap_mknod",
  "cap_lease",
  "cap_audit_write",
  "cap_audit_control",
  "cap_setfcap",
  "cap_mac_override",
  "cap_mac_admin",
  "cap_syslog",
  "cap_wake_alarm",
  "cap_block_suspend",
  "cap_audit_read"
  };


void printcaps(void){
  int i, j;
  cap_t cap;
  cap_value_t cv;
  cap_flag_value_t cap_flags_value;
  struct {
    const char *str;
    cap_flag_t flag;
    } flags[3] = {
    {"EFFECTIVE", CAP_EFFECTIVE},
    {"PERMITTED", CAP_PERMITTED},
    {"INHERITABLE", CAP_INHERITABLE}
    };

  cap=cap_get_proc();

  for(j=0; j<3; ++j){
    printf("%s: ", flags[j].str);
    for(i=0; i<CAP_LAST_CAP+1; i++){
      cap_from_name(cap_name[i], &cv);
      cap_get_flag(cap, cv, flags[j].flag, &cap_flags_value);
      if(cap_flags_value==CAP_SET)
        printf("%s ", cap_name[i]);
      }
    printf("\n");
    }

  cap_free(cap);
  }



/*
#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))


int main(int argc, char *argv[]){
  cap_t cap;
  cap_value_t cap_list[]={CAP_IPC_LOCK, CAP_SYS_NICE, CAP_SYS_RAWIO};

  if(!argv[1]){
    fprintf(stderr, "Usage: %s <command>\n", argv[0]);
    exit(1);
    }

  cap=cap_get_proc();
  if(cap_set_flag(cap, CAP_EFFECTIVE, ARRLEN(cap_list), cap_list, CAP_SET))
    fprintf(stderr, "cap_set_flag\n");
  if(cap_set_flag(cap, CAP_INHERITABLE, ARRLEN(cap_list), cap_list, CAP_SET))
    fprintf(stderr, "cap_set_flag\n");
  if(cap_set_proc(cap))
    fprintf(stderr, "cap_set_proc\n");
  cap_free(cap);

prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_IPC_LOCK, 0, 0);
prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_SYS_NICE, 0, 0);
//prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_SYS_RAWIO, 0, 0);

  printf("ioperm -> %d\n", ioperm(ORGEL_IO_PORTS_BASE, ORGEL_IO_PORTS_LEN, 1));
  SELECT_MODULE(0);
  MODULE_SET_REG(0);
  MODULE_READ();

//  cap=cap_get_proc();
//  if(cap_set_flag(cap, CAP_EFFECTIVE, ARRLEN(cap_list), cap_list, CAP_CLEAR))
//    fprintf(stderr, "cap_set_flag\n");
//  if(cap_set_flag(cap, CAP_PERMITTED, ARRLEN(cap_list), cap_list, CAP_CLEAR))
//    fprintf(stderr, "cap_set_flag\n");
//  if(cap_set_proc(cap))
//    fprintf(stderr, "cap_set_proc\n");
//  cap_free(cap);

  ++argv;
  execv(argv[0], argv);

  fprintf(stderr, "Cannot exec %s: %s\n", argv[0], strerror(errno));
  exit(1);
  }
*/

