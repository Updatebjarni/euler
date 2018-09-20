#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"cquencer.spec.c"

typedef struct cquencer_module{
  module;
  int time;
  }cquencer_module;

static void tick(module *_m, int elapsed){
  cquencer_module *m=(cquencer_module *)_m;
  if(INPUT(m)->ticksperbeat.connection){
    int32_t ticksperbeat=INPUT(m)->ticksperbeat.connection->value;
    
    m->time+=elapsed;
    //m->output.out_terminal.value.int32=value;
    }
  }

class cquencer_class;

static module *create(char **argv){
  cquencer_module *m;
  m=malloc(sizeof(cquencer_module));
  default_module_init((module *)m, &cquencer_class);
  m->time=0;
  return (module *)m;
  }

class cquencer_class={
  "cquencer",                // char *name
  "A simple 8 step sequencer", // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
