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

char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

int main(int argc, char *argv[]){
  char *line, *histfile=0;

  orgelperm_argv=malloc(sizeof(argv[0])*(argc+2));
  memcpy(orgelperm_argv+1, argv, sizeof(argv[0])*(argc+1));
  orgelperm_argv[0]="/usr/bin/orgelperm";
  signal(SIGSEGV, require_orgelperm);
  SELECT_MODULE(0);

  start_rt();
  module *mog=create_module("mog", 0);
  run_module(mog);

  asprintf(&histfile, "%s/.euler_history", getenv("HOME"));
  read_history(histfile);

  while((line=readline("euler> "))){
    if(strcspn(line, " \t")){
      add_history(line);
      write_history(histfile);
      history_truncate_file(histfile, 500);
      run_cmdline(line);
      }
    free(line);
    }

  stop_rt();

  return 0;
  }
