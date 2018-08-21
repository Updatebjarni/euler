#include<string.h>
#include<stdio.h>
#include"orgel.h"

void cmd_quit(char **);
extern const char help_quit[];
void cmd_help(char **);
const char help_help[]="Hjälp! Hjälp!\n";

struct command commands[]={
  {"q", cmd_quit, help_quit},
  {"quit", cmd_quit, help_quit},
  {"exit", cmd_quit, help_quit},
  {"avsluta", cmd_quit, help_quit},
  {"h", cmd_help, help_help},
  {"hjälp", cmd_help, help_help},
  {"help", cmd_help, help_help},
  {"?", cmd_help, help_help}
  };

int ncommands=sizeof(commands)/sizeof(commands[0]);

/*
FILE *columns(void){
  return popen("column -x | more", "w");
  }

int cmd_help(int again, char **argv){
  int (*f)(int, char **)=0;

  printf("Commands:\n");

  FILE *c=columns();
  for(int i=0; i<ncommands; i++){
    fprintf(c, (f==commands[i].function)?"/%s":"\n  %s", commands[i].name);
    f=commands[i].function;
    }
  fputc('\n', c);
  pclose(c);

  return 0;
  }
*/
void cmd_help(char **cmdline){
  if(cmdline[1]){
    for(int i=0; i<ncommands; ++i){
      if(!strcmp(commands[i].name, cmdline[1])){
        if(commands[i].help)printf("%s", commands[i].help);
        else printf("Ingen hjälp finns om kommandot.\n");
        }
      }
    return;
    }
  printf("Följande kommandon finns:\n");
  for(int i=0; i<ncommands; ++i){
    if(!(i%3))putchar('\n');
    printf("%20s", commands[i].name);
    }
  printf("\n\nSkriv \"hjälp <kommando>\" för hjälp om ett kommando.\n");
  }

void (*find_command(char *name))(char **){
  for(int i=0; i<ncommands; ++i)
    if(!strcmp(commands[i].name, name))return commands[i].function;
  return 0;
  }
