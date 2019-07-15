#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include"../orgel-io.h"

#define MOG_MODULE_NUMBER 16

char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

int main(int argc, char *argv[]){
  long chan;
  char *p;

  if(argc!=2 || ((chan=strtol(argv[1], &p, 0)), *p) || chan>2 || chan<0){
    fprintf(stderr, "Usage: bsselect <channel (0-2)>\n");
    return 0;
    }

  orgelperm_argv=malloc(sizeof(argv[0])*(argc+2));
  memcpy(orgelperm_argv+1, argv, sizeof(argv[0])*(argc+1));
  orgelperm_argv[0]="/usr/bin/orgelperm";
  signal(SIGSEGV, require_orgelperm);

  SELECT_MODULE(MOG_MODULE_NUMBER);
  MODULE_WRITE(0xC000|chan);

  return 0;
  }
