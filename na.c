#include<stdint.h>
#include"orgel.h"

#include"na.spec.c"


static void tick(module *m, int elapsed){
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;

  if(in->signal.state==CHANGED && in->control.state==CHANGED){
    int64_t result=(int64_t)(in->signal.connection->out_terminal.value.int32);
    result*=in->control.connection->out_terminal.value.int32;
    result>>=32;
    m->output.out_terminal.value.int32=result;
    }
  }

static int init(module *m, char **argv){
  return 0;
  }

class na_class={
  "na",                      // char *name
  "Numerical attenuverter",  // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  init,                      // int (*init)(module *, char **)
  0                          // int create_counter
  };
