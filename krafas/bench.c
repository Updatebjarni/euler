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

volatile int alarm_happened;

void sigalrm(int foo){
  alarm_happened=1;
  }

int main(int argc, char *argv[]){
  struct termios told, tnew;
  struct pollfd pofd[2];
struct itimerval itv;
struct sigaction sa;
long count;

  tcgetattr(0, &told);
  tnew=told;
  tnew.c_lflag&=~(ICANON|ECHO|ISIG);
  tnew.c_cc[VMIN]=1; tnew.c_cc[VTIME]=0;
  tcsetattr(0, TCSANOW, &tnew);

sigemptyset(&sa.sa_mask);
sa.sa_handler=sigalrm;
sa.sa_flags=0;
sigaction(SIGALRM, &sa, 0);

itv.it_interval.tv_sec=1;
itv.it_interval.tv_usec=0;
itv.it_value.tv_sec=1;
itv.it_value.tv_usec=0;

setitimer(ITIMER_REAL,  &itv, 0);

  pofd[0].fd=0; pofd[0].events=POLLIN;

  while(poll(pofd, 1, 0)!=-1){
    if(pofd[0].revents&POLLIN){
      int c=getchar();
      if(c=='q')break;
      }
    while(!alarm_happened)++count;
    alarm_happened=0;
    printf("count: %ld\n", count);
    count=0;
    }

  tcsetattr(0, TCSANOW, &told);

  return 0;
  }
