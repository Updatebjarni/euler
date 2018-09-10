#define _GNU_SOURCE

#include<errno.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
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
void cmd_show(char **);
extern const char help_show[];
void cmd_config(char **);
const char help_config[]="Configure a module.\n"
                         "Usage: config <module> [<parameter> ...]\n";
void cmd_j(char **);

void cmd_run(char **);
extern const char help_run[];
void cmd_stop(char **);
extern const char help_stop[];
void cmd_connect(char **);
extern const char help_connect[];
void cmd_set(char **);
const char help_set[]="Set an input to a constant value.\n"
                      "Usage: set <input jack> <value>\n";
void cmd_load(char **);
const char help_load[]="Executes commands from a file.\n";
void cmd_echo(char **);
const char help_echo[]="Prints a message to the terminal.\n";
void cmd_debug(char **);
extern const char help_debug[];

struct command commands[]={
  {"quit", cmd_quit, help_quit},
  {"q", cmd_quit, help_quit},
  {"exit", cmd_quit, help_quit},
  {"help", cmd_help, help_help},
  {"h", cmd_help, help_help},
  {"?", cmd_help, help_help},
  {"lsmod", cmd_lsmod, help_lsmod},
  {"lsclass", cmd_lsclass, help_lsclass},
  {"create", cmd_create, help_create},
  {"show", cmd_show, help_show},
  {"config", cmd_config, help_config},
  {"j", cmd_j, 0},  {"k", cmd_j, 0},
  {"run", cmd_run, help_run},
  {"connect", cmd_connect, help_connect},
  {"con", cmd_connect, help_connect},
  {"set", cmd_set, help_set},
  {"load", cmd_load, help_load},
  {"echo", cmd_echo, help_echo},
  {"debug", cmd_debug, help_debug}
  };

int ncommands=sizeof(commands)/sizeof(commands[0]);

FILE *columns(void){
  return popen("column -x | more", "w");
  }

void cmd_quit(char **cmdline){
  stop_rt();
  exit(0);
  }

const char help_quit[]="Quits the program.\n";

void cmd_echo(char **argv){
  while(*++argv){printf("%s ", *argv);}
  putchar('\n');
  }

char **tokenise(char *str, char *sep){
  int i=0;
  char **toklist=malloc(sizeof(char *));
  toklist[0]=strtok(str, sep);
  while(toklist[i]){
    ++i;
    toklist=realloc(toklist, sizeof(char *)*(i+1));
    toklist[i]=strtok(NULL, sep);
    }
  return toklist;
  }

void run_cmdline(char *line){
  char **toklist=tokenise(line, " \t");
  if(!toklist[0])return;
  void (*cmdfunc)(char **)=find_command(toklist[0]);
  if(cmdfunc)cmdfunc(toklist);
  else printf("Unknown command: %s\n", toklist[0]);
  free(toklist);
  }

void cmd_load(char **argv){
  if(!argv[1]){
    printf("No filename given.\n");
    return;
    }
  FILE *f=fopen(argv[1], "r");
  if(!f){
    printf("Could not open file.\n");
    return;
    }
  char *line=0;
  size_t len=0;
  errno=0;
  while(getline(&line, &len, f)!=-1){
    if(line[strlen(line)-1]=='\n')
      line[strlen(line)-1]=0;
    run_cmdline(line);
    }
  if(errno)perror(argv[1]);
  fclose(f);
  }

void cmd_set(char **argv){
  if(!argv[1] || !argv[2]){
    printf("Too few parameters.\n");
    return;
    }
  jack *j=find_jack(argv[1], DIR_IN);
  if(!j){
    printf("Input jack not found.\n");
    return;
    }
  if(!is_terminal(j)){
    printf("The jack is not a terminal.\n");
    return;
    }
  long n;
  char *end;
  n=strtol(argv[2], &end, 0);
  if(!*argv[2] || *end){
    printf("Invalid value \"%s\".\n", argv[2]);
    return;
    }
  jack *constant;
  if(j->in_terminal.connection){
    if(j->in_terminal.connection->out_terminal.parent_module){
      printf("The jack is already connected.\n");
      return;
      }
    constant=j->in_terminal.connection;
    }
  else constant=malloc(sizeof(jack));
  constant->type=j->type;
  if(j->type==TYPE_BOOL)
    constant->out_terminal.value.bool=n;
  else
    constant->out_terminal.value.int32=n;
  constant->out_terminal.parent_module=0;
  constant->out_terminal.changed=1;
  constant->out_terminal.connections=malloc(sizeof(jack *));
  constant->out_terminal.connections[0]=j;
  constant->out_terminal.nconnections=1;
  LOCK_MODULES();
  j->in_terminal.connection=constant;
  UNLOCK_MODULES();
  }

void cmd_j(char **argv){
  jack *j=find_jack(argv[1], (argv[0][0]=='j')?DIR_IN:DIR_OUT);
  if(!j){
    printf("not found\n");
    return;
    }
  switch(j->type){
    case TYPE_EMPTY:
      printf("type empty\n");
      break;
    case TYPE_BOOL:
      printf("type bool\n");
      break;
    case TYPE_INT32:
      printf("type int32\n");
      break;
    case TYPE_KEY_EVENTS:
      printf("type key_events\n");
      break;
    case TYPE_ARRAY:
      printf("type array\n");
      break;
    case TYPE_BUNDLE:
      printf("type bundle\n");
      break;
    }
  }

void cmd_config(char **cmdline){
  if(!cmdline[1]){
    printf(help_config);
    return;
    }
  module *m=find_module(cmdline[1]);
  if(!m){
    printf("No module named\"%s\".\n", cmdline[1]);
    return;
    }
  if(!m->config){
    printf("No configuration available for module \"%s\".\n", cmdline[1]);
    return;
    }
  m->config(m, cmdline+2);
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
