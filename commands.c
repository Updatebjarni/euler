#define _GNU_SOURCE

#include<string.h>
#include<stdio.h>
#include"orgel.h"

void cmd_quit(char **);
extern const char help_quit[];
void cmd_help(char **);
const char help_help[]="Help! Help!\n";
void cmd_lsmod(char **);
extern const char help_lsmod[];
void cmd_lsclass(char **);
extern const char help_lsclass[];
void cmd_create(char **);
extern const char help_create[];

struct command commands[]={
  {"quit", cmd_quit, help_quit},
  {"q", cmd_quit, help_quit},
  {"exit", cmd_quit, help_quit},
  {"help", cmd_help, help_help},
  {"h", cmd_help, help_help},
  {"?", cmd_help, help_help},
  {"lsmod", cmd_lsmod, help_lsmod},
  {"lsclass", cmd_lsclass, help_lsclass},
  {"create", cmd_create, help_create}
  };

int ncommands=sizeof(commands)/sizeof(commands[0]);

FILE *columns(void){
  return popen("column -x | more", "w");
  }

void cmd_help(char **cmdline){
  if(cmdline[1]){
    for(int i=0; i<ncommands; ++i){
      if(!strcmp(commands[i].name, cmdline[1])){
        if(commands[i].help)printf("%s", commands[i].help);
        else printf("There is no help for this command.\n");
        }
      }
    return;
    }
  printf("The following commands are available:\n");

  void (*f)(char **)=0;
  FILE *c=columns();
  for(int i=0; i<ncommands; i++){
    fprintf(c, (f==commands[i].function)?"/%s":"\n  %s", commands[i].name);
    f=commands[i].function;
    }
  fputc('\n', c);
  pclose(c);

  printf("\nType \"help <command>\" for help on a command.\n");
  }

void (*find_command(char *name))(char **){
  for(int i=0; i<ncommands; ++i)
    if(!strcmp(commands[i].name, name))return commands[i].function;
  return 0;
  }
