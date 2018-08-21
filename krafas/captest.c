#include<stdio.h>
#include<sys/capability.h>


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
  "cap_syslog"
  };

int main(){
  int i, j;
  cap_t cap;
  cap_value_t cap_list[CAP_LAST_CAP+1];
  cap_flag_t cap_flags;
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

    for (i=0; i < CAP_LAST_CAP + 1; i++) {
      cap_from_name(cap_name[i], &cap_list[i]);
      printf("%-20s %d\t\t", cap_name[i], cap_list[i]);
      printf("flags: \t\t");
      for (j=0; j < 3; j++) {
	cap_get_flag(cap, cap_list[i], flags[j].flag, &cap_flags_value);
	printf(" %s %-4s ", flags[j].str, (cap_flags_value == CAP_SET) ? "OK" : "NOK");
	}
      printf("\n");
      }

  cap_free(cap);

  }
