#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"gdelay.spec.c"

typedef struct gdelay_module{
  module;
  int maxticks; // Number of ticks delay
  int32_t *gate;
  }gdelay_module;

static void tick(module *_m, int elapsed){
  gdelay_module *m=(gdelay_module *)_m;
  if(INPUT(m)->gate.connection && INPUT(m)->delay.connection){
<<<<<<< HEAD
    int32_t gate=(INPUT(m)->gate.connection->value);
    int32_t delay=(INPUT(m)->delay.connection->value);
=======
    int32_t gate=INPUT(m)->gate.connection->value;
    int32_t delay=INPUT(m)->delay.connection->value;
>>>>>>> a31e99f86c5875391d495e648b6ebd557c7f29cf

    if (delay > m->maxticks)
      delay = m->maxticks;

    if (delay < 0)
      delay = 0;
    
    for (int i=delay; i>0; i--){
      m->gate[i]=m->gate[i-1];
      }

    m->gate[0]=gate;
    
<<<<<<< HEAD
    OUTPUT(m).bool_value=m->gate[delay];
=======
    //m->output.out_terminal.value.bool=m->gate[delay];
    OUTPUT(m).int32_value=m->gate[delay];
>>>>>>> a31e99f86c5875391d495e648b6ebd557c7f29cf
    }
  }

class gdelay_class;

static module *create(char **argv){
  gdelay_module *m;

  m=malloc(sizeof(gdelay_module));
  default_module_init((module *)m, &gdelay_class);
  m->maxticks=10000;
  m->gate=malloc(sizeof(int32_t)*m->maxticks);
  for (int i = 0; i<1000*2; i++){
    m->gate[i]=0;
    }
  return (module *)m;
  }

class gdelay_class={
  "gdelay",                     // char *name
  "Gate delay",          // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
