#define _GNU_SOURCE

#include<alsa/asoundlib.h>
#include<stdio.h>

#define NAME "midi.c"

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

#include"orgel.h"
#include"orgel-io.h"

#define PERIOD 1500000L

extern struct orgel_module module_mog;

volatile int quit;

int pifd[2];

void *rt_thread(void *data){
  struct timespec next;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1){
    if(quit)return 0;

module_mog.tick(1);

    next.tv_nsec+=PERIOD;
    while(next.tv_nsec>=1000000000){
      next.tv_sec++;
      next.tv_nsec-=1000000000;
      }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    }
  }

char *tangentnamn[]={
  "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E"
  };

char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

int main(int argc, char *argv[]){
  struct sched_param sched;
  pthread_attr_t attr;
  pthread_t thread;
  struct key_event e;
  struct termios told, tnew;
  struct pollfd pofd[2];

  snd_seq_t *seq;
  int in_port, client, queue;
  snd_seq_addr_t port={.client=128, .port=0};
  snd_seq_port_info_t *pinfo;
  snd_seq_event_t ev;

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

  pipe(pifd);

  pofd[0].fd=0; pofd[0].events=POLLIN;
  pofd[1].fd=pifd[0]; pofd[1].events=POLLIN;

  SELECT_MODULE(16);

module_mog.init(pifd[1]);

  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 1024*1024);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  sched.sched_priority=80;
  pthread_attr_setschedparam(&attr, &sched);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_create(&thread, &attr, rt_thread, NULL);

  while(poll(pofd, 2, -1)!=-1){
    if(pofd[0].revents&POLLIN){
  int c=getchar();
  if(c=='q')break;
  }
    if(pofd[1].revents&POLLIN){
      read(pifd[0], &e, sizeof(e));
      printf("%s%d %s", tangentnamn[e.key%12], e.key/12, e.state?"upp":"ner");
      putchar('\n');
      }
    }

  tcsetattr(0, TCSANOW, &told);

  quit=1;

  pthread_join(thread, NULL);

  return 0;
  }
