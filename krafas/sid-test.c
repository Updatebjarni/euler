#define _GNU_SOURCE

#include<stdlib.h>
#include<stdio.h>
#include<sys/io.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>

#include"orgel.h"
#include"orgel-io.h"


char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

int main(int argc, char *argv[]){
  unsigned short w=0;

  orgelperm_argv=malloc(sizeof(argv[0])*(argc+2));
  memcpy(orgelperm_argv+1, argv, sizeof(argv[0])*(argc+1));
  orgelperm_argv[0]="/usr/bin/orgelperm";
  signal(SIGSEGV, require_orgelperm);

  SELECT_MODULE(1);
  while(1){
    MODULE_WRITE(0x0410);
    MODULE_READ();
//    w+=0x2000;
    }

  return 0;
  }
