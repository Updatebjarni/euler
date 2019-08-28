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

#include"all_modules.c"

int rt_lock, rt_lock_count;

module **all_modules;
int nmodules;

static module **running;
static int nrunning;

module *find_module(char *name){
  for(int i=0; i<nmodules; ++i)
    if(!strcmp(all_modules[i]->name, name))return all_modules[i];
  return 0;
  }

const char help_debug[]="Print some internal state useful for debugging.\n"
                        "Usage: debug <module>\n";

void cmd_debug(char **argv){
  if(!argv[1]){
    printf("No module name given.\n");
    return;
    }
  module *m=find_module(argv[1]);
  if(!m){
    printf("Not found.\n");
    return;
    }
  m->debug(m);
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

void base_module_init(module *m, class *c,
                      jack *input, jack *input_template,
                      jack *output, jack *output_template){
  m->type=c;
  m->name=0;
  m->tick=c->tick;
  m->destroy=c->destroy;
  m->config=c->config;
  m->debug=c->debug;
  m->reset=c->reset;
  m->plugstatus=c->plugstatus;
  create_jack(input, input_template, m);
  m->input_ptr=input;
//fprintf(stderr, "foere 3\n");
//free(m->input_ptr);
//fprintf(stderr, "efter 3\n");
  create_jack(output, output_template, m);
  m->output_ptr=output;
  m->last_updated=0;
  }

void cmd_shutup(char **argv){
  for(int i=0; i<nmodules; ++i){
    stop_module(all_modules[i]);
    if(all_modules[i]->type->is_static && all_modules[i]->reset)
      all_modules[i]->reset(all_modules[i]);
    }
  }

const char help_shutup[]="Makes the synth modules shut up.\n";

void clear_all(void){
  cmd_shutup(0);
  for(int i=0; i<nmodules;)
    if(all_modules[i]->type->is_static)
      ++i;
    else
      destroy_module(all_modules[i]);
  for(class **c=all_classes; *c; ++c)
    if(!((*c)->is_static))
      (*c)->create_counter=0;
  }

void cmd_clear(char **argv){
  clear_all();
  }

const char help_clear[]="Stops and unloads all modules.\n";

module *create_module(char *class_name, char **argv){
  module *m;
  class *c=find_class(class_name);
  if(!c)return 0;
  if(c->is_static && c->create_counter)
    return find_module(class_name);

  m=c->create(argv);
  if(!m)return 0;

  ++(c->create_counter);
  if(c->is_static)
    m->name=strdup(class_name);
  else
    asprintf(&(m->name), "%s-%d", class_name, c->create_counter);
  LOCK_NEST();
  all_modules=realloc(all_modules, sizeof(all_modules[0])*(nmodules+1));
  all_modules[nmodules]=m;
  ++nmodules;
  LOCK_UNNEST();
  return m;
  }

const char help_create[]="Create a new module.\n";

void cmd_create(char **argv){
  module *m;
  char *classname=0, *modname=0, **paren=0;
  ++argv;
  if(!*argv){
    printf("Usage: create <class-name(options)> [as <name>]\n");
    return;
    }
  if(!find_class(*argv)){
    printf("No such class: %s\n", *argv);
    return;
    }
  classname=*argv;
  ++argv;
  if(*argv && !strcmp(*argv, "(") && parse_parens(&argv, &paren)==-1)
    goto syntax_error;
  if(*argv){
    if(strcmp(*argv, "as") || !*++argv)goto syntax_error;
    if(find_module(*argv)){
      printf("Name \"%s\" already exists.\n", *argv);
      return;
      }
    modname=*argv;
    ++argv;
    }
  if(!(m=create_module(classname, paren))){
    printf("Failed to create module \"%s\".\n", modname?modname:classname);
    return;
    }
  if(modname){
    free(m->name);
    m->name=strdup(modname);
    }
  return;

  syntax_error:
  printf("Syntax error.\n");
  return;
  }

void destroy_module(module *m){
  disconnect_jack(m->input_ptr);
  disconnect_jack(m->output_ptr);
  LOCK_NEST();
  for(int i=0; i<nmodules; ++i)
    if(all_modules[i]==m){
      all_modules[i]=all_modules[--nmodules];
      break;
      }
  LOCK_UNNEST();
  if(m->destroy)m->destroy(m);
  destroy_jack(m->input_ptr);
  destroy_jack(m->output_ptr);
  free(m->name);
  free(m);
  }

void run_module(module *m){
  LOCK_NEST();
  running=realloc(running, sizeof(module[nrunning+1]));
  running[nrunning]=m;
  ++nrunning;
  LOCK_UNNEST();
  }

const char help_run[]="Set a module as running. Usage: run <module>\n";

void cmd_run(char **argv){
  module *m=find_module(argv[1]);
  if(!m){printf("not found\n"); return;}
  run_module(m);
  }

void stop_module(module *m){
  for(int i=0; i<nrunning; ++i)
    if(running[i]==m){
      LOCK_NEST();
      running[i]=running[--nrunning];
      running=realloc(running, sizeof(running[0])*nrunning);
      LOCK_UNNEST();
      return;
      }
  }

const char help_stop[]="Stop a module, so that it is no longer running.\n"
                       "Usage: stop <module>\n";

void cmd_stop(char **argv){
  module *m=find_module(argv[1]);
  if(!m){printf("not found\n"); return;}
  stop_module(m);
  }

static volatile int quit;
static int oddeven;

static void jack_depend(jack *j){
  if((j->type==TYPE_EMPTY) || is_terminal(j)){
    if(!(j->connection))return;
    module *m=j->connection->parent_module;
    if(!m)return;
    if(m->last_updated!=oddeven){
      jack_depend(m->input_ptr);
      m->tick(m, 1);
      m->last_updated=oddeven;
      }
    }
  else{
    if(j->type==TYPE_ARRAY){
      jack *e=j->array;
      int size=j->array->len+1;
      for(int i=0; i<j->len; ++i)
        jack_depend(e+i*size);
      }
    else if(j->type==TYPE_BUNDLE){
      for(int i=1; i<=j->len; ++i)
        jack_depend(j+i);
      }
    }
  }

static void *rt_thread(void *data){
  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1){
    if(quit)return 0;

    if(!TRY_LOCK_EXCL()){
      for(int i=0; i<nrunning; ++i){
        jack_depend(running[i]->input_ptr);
        running[i]->tick(running[i], 1);
        running[i]->last_updated=oddeven;
        }
      UNLOCK();
      }
    oddeven=1-oddeven;

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

#ifndef NO_HARDWARE
  if(mlockall(MCL_CURRENT|MCL_FUTURE)<0){
    fprintf(stderr, "mlockall(): %m\n");
    exit(1);
    }
#endif

  pthread_attr_init(&attr);
#ifndef NO_HARDWARE
  pthread_attr_setstacksize(&attr, 1024*1024);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_getschedparam(&attr, &sched);
  sched.sched_priority=80;
  pthread_attr_setschedparam(&attr, &sched);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
#endif
  pthread_create(&thread, &attr, rt_thread, NULL);
  pthread_attr_destroy(&attr);
  }

void stop_rt(void){
  quit=1;
  pthread_join(thread, NULL);
  }

void cmd_show(char **argv){
  module *m=find_module(argv[1]);
  if(!m){
    printf("No such module.\n");
    return;
    }
  printf("%s module \"%s\" (of class \"%s\"):\n",
         m->type->is_static?"Static":"Dynamic", m->name, m->type->name);
  if(m->input_ptr->type!=TYPE_EMPTY){
    printf("input: ");
    show_jack(m->input_ptr, DIR_IN, 0);
    }
  if(m->output_ptr->type!=TYPE_EMPTY){
    printf("output: ");
    show_jack(m->output_ptr, DIR_OUT, 0);
    }
  if(m->output_ptr->type==TYPE_EMPTY)
    printf("(This module provides no output)\n");
  if(m->input_ptr->type==TYPE_EMPTY)
    printf("(This module takes no input)\n");
  }

const char help_show[]=
  "show <module> shows information about a module.";
