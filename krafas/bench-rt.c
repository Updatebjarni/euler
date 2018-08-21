#define _GNU_SOURCE

#include<stdio.h>
#include<time.h>
#include<pthread.h>
#include<sched.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/io.h>
#include<sys/mman.h>
#include<unistd.h>
#include<termios.h>
#include<sys/poll.h>
#include<sys/time.h>
#include<signal.h>
#include<string.h>

#include"orgel-io.h"

#define PERIOD 1000000L

volatile int quit;
volatile int ops=900;

int pifd[2];

void *rt_thread(void *data){
  struct timespec next;
int count;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1){
    if(quit)return 0;

for(count=0; count<ops; ++count)SELECT_MODULE(0);

    next.tv_nsec+=PERIOD;
    while(next.tv_nsec>=1000000000){
      next.tv_sec++;
      next.tv_nsec-=1000000000;
      }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    }
  }

char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

int main(int argc, char *argv[]){
  struct sched_param sched;
  pthread_attr_t attr;
  pthread_t thread;
  struct termios told, tnew;
  struct pollfd pofd[2];

  orgelperm_argv=malloc(sizeof(argv[0])*(argc+2));
  memcpy(orgelperm_argv+1, argv, sizeof(argv[0])*(argc+1));
  orgelperm_argv[0]="/usr/bin/orgelperm";
  signal(SIGSEGV, require_orgelperm);
  SELECT_MODULE(0);

  tcgetattr(0, &told);
  tnew=told;
  tnew.c_lflag&=~(ICANON|ECHO|ISIG);
  tnew.c_cc[VMIN]=1; tnew.c_cc[VTIME]=0;
  tcsetattr(0, TCSANOW, &tnew);

  if(mlockall(MCL_CURRENT|MCL_FUTURE)<0){
    fprintf(stderr, "mlockall(): %m\n");
    exit(1);
    }

  pofd[0].fd=0; pofd[0].events=POLLIN;

  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 1024*1024);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  sched.sched_priority=80;
  pthread_attr_setschedparam(&attr, &sched);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_create(&thread, &attr, rt_thread, NULL);

  while(poll(pofd, 1, -1)!=-1){
    if(pofd[0].revents&POLLIN){
      int c=getchar();
      if(c=='q')break;
      if(c=='+')ops+=10;
      if(c=='-')ops-=10;
      printf("ops=%d\n", ops);
      }
    }

  tcsetattr(0, TCSANOW, &told);

  quit=1;

  pthread_join(thread, NULL);

  return 0;
  }
