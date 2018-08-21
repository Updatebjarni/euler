#define _GNU_SOURCE

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<pthread.h>
#include<sched.h>
#include<sys/mman.h>
#include"orgel.h"

#define PERIOD 1000000L

extern class mog_class, sid_class;

class *all_classes[]={
  &mog_class,
//  &sid_class,
  0
  };

module **all_modules;
int nmodules;

static module **running;
static int nrunning;

/*
module *find_module(char *name){
  for(int i=0; i<nmodules; ++i)
    if(loaded_modules[i]->
  }
*/

class *find_class(char *name){
  for(class **c=all_classes; *c; ++c)
    if(!strcmp(name, (*c)->name))return *c;
  return 0;
  }

module *create_module(char *class_name){
  class *c=find_class(class_name);
  if(!c || (c->is_static && c->create_counter))return 0;
  module *m=c->create();
  ++(c->create_counter);
  asprintf(&(m->name), "%s-%d", class_name, c->create_counter);
  return m;
  }

void run_module(module *m){
  running=realloc(running, sizeof(module[nrunning+1]));
  running[nrunning]=m;
  ++nrunning;
  }

void stop_module(module *m){
  // ***
  }

static volatile int quit;

static void *rt_thread(void *data){
  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1){
    if(quit)return 0;

//  module_mog.tick(1);

    next.tv_nsec+=PERIOD;
    while(next.tv_nsec>=1000000000){
      next.tv_sec++;
      next.tv_nsec-=1000000000;
      }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    }
  }

static pthread_t thread;

void start_rt(void){
  struct sched_param sched;
  pthread_attr_t attr;

  if(mlockall(MCL_CURRENT|MCL_FUTURE)<0){
    fprintf(stderr, "mlockall(): %m\n");
    exit(1);
    }

  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 1024*1024);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_getschedparam(&attr, &sched);
  sched.sched_priority=80;
  pthread_attr_setschedparam(&attr, &sched);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_create(&thread, &attr, rt_thread, NULL);
  pthread_attr_destroy(&attr);
  }

void stop_rt(void){
  quit=1;
  pthread_join(thread, NULL);
  }
