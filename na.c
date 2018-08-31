#include<math.h>
#include<stdint.h>
#include"orgel.h"

#include"na.spec.c"


static void tick(module *m, int elapsed){
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;

  if(in->signal.connection && in->control.connection){
    int64_t result=(int64_t)(in->signal.connection->out_terminal.value.int32);
    result*=pow(2, 1.0*in->control.connection->out_terminal.value.int32/OCTAVE);
    if(result>INT32_MAX)result=INT32_MAX;
    if(result<INT32_MIN)result=INT32_MIN;
    m->output.out_terminal.value.int32=result;
    }
  }

/*
static int init(module *m, char **argv){

  }
*/

class na_class={
  "na",                      // char *name
  "Numerical attenuverter",  // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  0,                         // int (*init)(module *, char **)
  0                          // int create_counter
  };
