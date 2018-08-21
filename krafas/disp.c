#define _GNU_SOURCE

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>

#include"ui.h"


static void tput(char *fmt, ...){
  va_list ap;
  char *s;
  FILE *p;

  asprintf(&s, "tput %s", fmt);
  fmt=s;
  va_start(ap, fmt);
  vasprintf(&s, fmt, ap);
  va_end(ap);
  free(fmt);

  p=popen(s, "w");
  pclose(p);

  free(s);
  }

void ui_curpos(int x, int y){
  tput("cup %d %d", y, x);
  }

void ui_rv_on(){
  tput("rev");
  }

void ui_rv_off(){
  tput("sgr0");
  }

void ui_clear(){
  tput("clear");
  }

void ui_init(){
  tput("smcup");
  atexit(ui_exit);
  }

void ui_exit(){
  tput("rmcup");
  }

void ui_print_main(){
  ui_clear();
  ui_curpos(20, 4);
  printf("======== Kommandon ========\n");
  for(int i=0; ui_commands[i].key; ++i){
    ui_curpos(20, 5+i);
    printf("%c  %s\n", ui_commands[i].key, ui_commands[i].descr);
    }
  }
