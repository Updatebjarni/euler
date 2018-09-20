#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"sdelay.spec.c"

typedef struct sdelay_module{
  module;
  int maxticks;
  int32_t *signal;
  }sdelay_module;

static void tick(module *_m, int elapsed){
  sdelay_module *m=(sdelay_module *)_m;
  if(INPUT(m)->signal.connection && INPUT(m)->delay.connection){
    int32_t delay=INPUT(m)->delay.connection->value;
    int32_t signal=INPUT(m)->signal.connection->value;

    if (delay > m->maxticks)
      delay = m->maxticks;

    if (delay < 0)
      delay = 0;
                   
    for (int i=delay; i>0; i--){
      m->signal[i]=m->signal[i-1];
      }

    m->signal[0]=signal;
    
<<<<<<< HEAD
    OUTPUT(m).bool_value=m->signal[delay];
=======
    OUTPUT(m).int32_value=m->signal[delay];
>>>>>>> a31e99f86c5875391d495e648b6ebd557c7f29cf
    }
  }

class sdelay_class;

static module *create(char **argv){
  sdelay_module *m;

  m=malloc(sizeof(sdelay_module));
  default_module_init((module *)m, &sdelay_class);
  m->maxticks=10000;
  m->signal=malloc(sizeof(int32_t)*m->maxticks);
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
