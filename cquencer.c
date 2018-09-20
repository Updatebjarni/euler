#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"cquencer.spec.c"

typedef struct cquencer_module{
  module;
  int time; // Time since start
  int32_t *pitch; // Key value
  int32_t *value1;
  int32_t *value2;
  }cquencer_module;

static int32_t keytopitch(int key){
  int32_t base=0;//A440/2;
  return base+key*HALFNOTE;
}

static void tick(module *_m, int elapsed){
  cquencer_module *m=(cquencer_module *)_m;
  if(INPUT(m)->ticksperbeat.connection){
    int32_t ticksperbeat=INPUT(m)->ticksperbeat.connection->value;
    
    int pwl=m->time%(8*ticksperbeat);
    int step=pwl/ticksperbeat;
    pwl-=ticksperbeat*step;

    int gate=0;
    if (pwl<100)
      gate=1;
    
    OUTPUT(m)->pitch.int32_value=m->pitch[step];
    OUTPUT(m)->value1.int32_value=m->value1[step];
    OUTPUT(m)->value2.int32_value=m->value2[step];
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
  m->pitch=malloc(sizeof(int32_t));
  m->value1=malloc(sizeof(int32_t));
  m->value2=malloc(sizeof(int32_t));
  
  m->pitch[0]=keytopitch(0);
  m->pitch[1]=keytopitch(0);
  m->pitch[2]=keytopitch(2);
  m->pitch[3]=keytopitch(0);
  m->pitch[4]=keytopitch(3);
  m->pitch[5]=keytopitch(0);
  m->pitch[6]=keytopitch(5);
  m->pitch[7]=keytopitch(3);

  m->value1[0]=keytopitch(400);
  m->value1[1]=keytopitch(400);
  m->value1[2]=keytopitch(400);
  m->value1[3]=keytopitch(400);
  m->value1[4]=keytopitch(400);
  m->value1[5]=keytopitch(400);
  m->value1[6]=keytopitch(400);
  m->value1[7]=keytopitch(400);

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
