#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"pulse.spec.c"

typedef struct pulse_module{
  module;
  int time;
  }pulse_module;

static void tick(module *_m, int elapsed){
  pulse_module *m=(pulse_module *)_m;
  if(INPUT(m)->gate.connection && INPUT(m)->length.connection &&
     INPUT(m)->height.connection){

    if(INPUT(m)->gate.connection->value){
      if(m->time=0){
	m->time=INPUT(m)->length.connection->value;
        }
      }

    if(m->time>0){
      OUTPUT(m).int32_value=INPUT(m)->height.connection->value;
      } else {
      OUTPUT(m).int32_value=0;
      }
    }
  }

class pulse_class;

static module *create(char **argv){
  pulse_module *m;

  m=malloc(sizeof(pulse_module));
  default_module_init((module *)m, &pulse_class);
  m->time=0;
  return (module *)m;
  }

class pulse_class={
  "pulse",                     // char *name
  "Generates an output signal while gate is on", // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
