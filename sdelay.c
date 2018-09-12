#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"sdelay.spec.c"

typedef struct sdelay_module{
  module;
  int ticks; // Number of ticks delay
  int32_t *signal;
  }sdelay_module;

static void tick(module *_m, int elapsed){
  sdelay_module *m=(sdelay_module *)_m;
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;
  if(in->signal.connection){
    int32_t signal=(in->signal.connection->out_terminal.value.bool);
  
    for (int i=m->ticks; i>0; i--){
      m->signal[i]=m->signal[i-1];
      }

    m->signal[0]=signal;
    
    m->output.out_terminal.value.bool=m->signal[m->ticks];
    }
  }

class sdelay_class;

static module *create(char **argv){
  sdelay_module *m;

  m=malloc(sizeof(sdelay_module));
  default_module_init((module *)m, &sdelay_class);
  m->ticks=1000;
  m->signal=malloc(sizeof(int32_t)*1000*2);
  // Maximum delay is currently hard coded
  for (int i = 0; i<1000*2; i++){
    m->signal[i]=0;
    }
  return (module *)m;
  }

class sdelay_class={
  "sdelay",                     // char *name
  "Signal delay",          // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
