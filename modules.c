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

int modules_lock, hardware_lock;

class *all_classes[]={
  &mog_class,
//  &sid_class,
  0
  };

module **all_modules;
int nmodules;

static module **running;
static int nrunning;

module *find_module(char *name){
  for(int i=0; i<nmodules; ++i)
    if(!strcmp(all_modules[i]->name, name))return all_modules[i];
  return 0;
  }

const char help_lsmod[]="Lists all loaded modules.\n";

void cmd_lsmod(char **cmdline){
  FILE *c=columns();
  for(int i=0; i<nmodules; ++i)
    fprintf(c, "%s (%s)\n", all_modules[i]->name, all_modules[i]->type->name);
  fputc('\n', c);
  pclose(c);
  }

const char help_lsclass[]="Lists all available module classes.\n";

void cmd_lsclass(char **cmdline){
  FILE *c=columns();
  for(int i=0; all_classes[i]; ++i)
    fprintf(c, "%s\n", all_classes[i]->name);
  fputc('\n', c);
  pclose(c);
  }

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
  if(c->is_static)
    m->name=strdup(class_name);
  else
    asprintf(&(m->name), "%s-%d", class_name, c->create_counter);
  LOCK_MODULES();
  all_modules=realloc(all_modules, sizeof(all_modules[0])*(nmodules+1));
  all_modules[nmodules]=m;
  ++nmodules;
  UNLOCK_MODULES();
  return m;
  }

const char help_create[]="Create a new module.\n";

void cmd_create(char **cmdline){
  module *m;
  if(!cmdline[1]){
    printf("Usage: create <class-name> [<instance-name>]\n");
    return;
    }
  if(!find_class(cmdline[1])){
    printf("No such class: %s\n", cmdline[1]);
    return;
    }
  if(cmdline[2]){
    if(find_module(cmdline[2])){
      printf("Name \"%s\" already exists.\n", cmdline[2]);
      return;
      }
    }
  if(!(m=create_module(cmdline[1]))){
    printf("Failed to create module.\n");
    return;
    }
  if(cmdline[2]){
    free(m->name);
    m->name=strdup(cmdline[2]);
    }
  }

void run_module(module *m){
  LOCK_MODULES();
  running=realloc(running, sizeof(module[nrunning+1]));
  running[nrunning]=m;
  ++nrunning;
  UNLOCK_MODULES();
  }

void stop_module(module *m){
  for(int i=0; i<nrunning; ++i)
    if(running[i]==m){
      LOCK_MODULES();
      running[i]=running[--nrunning];
      running=realloc(running, sizeof(running[0])*nrunning);
      UNLOCK_MODULES();
      return;
      }
  }

static volatile int quit;

static void *rt_thread(void *data){
  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1){
    if(quit)return 0;

    if(!TRY_LOCK_MODULES()){
      for(int i=0; i<nrunning; ++i)
        running[i]->tick(running[i], 1);
      UNLOCK_MODULES();
      }

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
