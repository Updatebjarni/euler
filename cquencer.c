#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"cquencer.spec.c"

typedef struct cquencer_module{
  module;
  int time; // Time since start
  }cquencer_module;

static void pstep(int input, cquencer_module *m){
  if (!INPUT(m)->step.INDEX(input)->pitch.connection) return;
  int32_t p = INPUT(m)->step.INDEX(input)->pitch.connection->value;
  OUTPUT(m)->pitch.int32_value=p;
}

static void v1step(int input, cquencer_module *m){
  if (!INPUT(m)->step.INDEX(input)->value1.connection) return;
  int32_t p = INPUT(m)->step.INDEX(input)->value1.connection->value;
  OUTPUT(m)->value1.int32_value=p;
}
  
static void v2step(int input, cquencer_module *m){
  if (!INPUT(m)->step.INDEX(input)->value2.connection) return;
  int32_t p = INPUT(m)->step.INDEX(input)->value2.connection->value;
  OUTPUT(m)->value2.int32_value=p;
}

static void tick(module *_m, int elapsed){
  cquencer_module *m=(cquencer_module *)_m;
  if(INPUT(m)->ticksperbeat.connection){
    int32_t ticksperbeat=INPUT(m)->ticksperbeat.connection->value;
    
    int pwl=m->time%(8*ticksperbeat);
    int step=pwl/ticksperbeat;
    pwl%=ticksperbeat;

    int gate=0;
    if (pwl<100)
      gate=1;
    
    //OUTPUT(m)->pitch.int32_value=m->pitch[step];
    pstep(step, m);
    v1step(step, m);
    v2step(step, m);
    OUTPUT(m)->gate.bool_value=gate;

    m->time+=elapsed;
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
