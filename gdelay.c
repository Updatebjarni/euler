#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"gdelay.spec.c"

typedef struct gdelay_module{
  module;
  int ticks; // Number of ticks delay
  int32_t *gate;
  }gdelay_module;

static void tick(module *_m, int elapsed){
  gdelay_module *m=(gdelay_module *)_m;
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;
  if(in->gate.connection){
    int32_t gate=(in->gate.connection->out_terminal.value.bool);
  
    for (int i=m->ticks; i>0; i--){
      m->gate[i]=m->gate[i-1];
      }

    m->gate[0]=gate;
    
    m->output.out_terminal.value.bool=m->gate[m->ticks];
    }
  }

class gdelay_class;

static module *create(char **argv){
  gdelay_module *m;

  m=malloc(sizeof(gdelay_module));
  default_module_init((module *)m, &gdelay_class);
  m->ticks=1000;
  m->gate=malloc(sizeof(int32_t)*1000*2);
  // Maximum delay is currently hard coded
  for (int i = 0; i<1000*2; i++){
    m->gate[i]=0;
    }
  return (module *)m;
  }

class gdelay_class={
  "gdelay",                     // char *name
  "Gate signal delay",          // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
