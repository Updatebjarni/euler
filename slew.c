#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"slew.spec.c"

typedef struct slew_module{
  module;
  int value;
  }slew_module;

static int min(int a, int b){
  return (a < b) ? a : b;
}

static void tick(module *_m, int elapsed){
  slew_module *m=(slew_module *)_m;
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;

  if(in->signal.connection && in->rate.connection){
    int32_t signal=(in->signal.connection->out_terminal.value.int32);
    int32_t rate=(in->rate.connection->out_terminal.value.int32);

    if (rate < 0) rate=0;
    
    if (m->value > signal)
      m->value-= min(m->value - signal, rate);

    if (m->value < signal)
      m->value+= min(signal - m->value, rate);

    m->output.out_terminal.value.int32=m->value;
    }
  }

class slew_class;

static module *create(char **argv){
  slew_module *m;

  m=malloc(sizeof(slew_module));
  default_module_init((module *)m, &slew_class);
  m->value=0;
  return (module *)m;
  }

class slew_class={
  "slew",                     // char *name
  "Portamento effect",        // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
