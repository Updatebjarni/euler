#define _GNU_SOURCE

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/io.h>
#include<unistd.h>
#include<termios.h>
#include<sys/poll.h>
#include<sys/time.h>
#include<signal.h>
#include<readline/readline.h>
#include<readline/history.h>

#include"orgel.h"
#include"orgel-io.h"


void cmd_quit(char **cmdline){
  stop_rt();
  exit(0);
  }

const char help_quit[]="Quits the program.\n";

char **tokenise(char *line){
  int i=0;
  char **toklist=malloc(sizeof(char *));
  toklist[0]=strtok(line, " \t");
  while(toklist[i]){
    ++i;
    toklist=realloc(toklist, sizeof(char *)*(i+1));
    toklist[i]=strtok(NULL, " \t");
    }
  return toklist;
  }

char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

int main(int argc, char *argv[]){
  char *line, **toklist=0;
  HIST_ENTRY *last;

  orgelperm_argv=malloc(sizeof(argv[0])*(argc+2));
  memcpy(orgelperm_argv+1, argv, sizeof(argv[0])*(argc+1));
  orgelperm_argv[0]="/usr/bin/orgelperm";
  signal(SIGSEGV, require_orgelperm);
  SELECT_MODULE(0);

  start_rt();
  module *mog=create_module("mog", 0);
  run_module(mog);

  while((line=readline("euler> "))){
    char **toklist=tokenise(line);
    if(toklist[0]){
      add_history(line);

      void (*cmdfunc)(char **)=find_command(toklist[0]);
      if(cmdfunc)cmdfunc(toklist);
      else printf("Unknown command: %s\n", toklist[0]);
      }

    free(toklist);
    free(line);
    }

  stop_rt();

  return 0;
  }
