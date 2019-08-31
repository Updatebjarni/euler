#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"sdelay.spec.c"

typedef struct sdelay_module{
  MODULE_BASE
  int maxticks;
  int32_t *signal;
  }sdelay_module;

static void tick(module *_m, int elapsed){
  sdelay_module *m=(sdelay_module *)_m;
  int32_t delay=m->input.delay.value;
  int32_t signal=m->input.signal.value;

  if (delay > m->maxticks)
    delay = m->maxticks;

  if (delay < 0)
    delay = 0;
                   
  for (int i=delay; i>0; i--){
    m->signal[i]=m->signal[i-1];
    }

  m->signal[0]=signal;
    
  m->output.value=m->signal[delay];
  set_output(&m->output);
  }

class sdelay_class;

static module *create(char **argv){
  sdelay_module *m;

  m=malloc(sizeof(sdelay_module));
  base_module_init(m, &sdelay_class);
  m->maxticks=10000;
  m->signal=malloc(sizeof(int32_t)*m->maxticks);
  for (int i = 0; i<1000*2; i++){
    m->signal[i]=0;
    }
  return (module *)m;
  }

class sdelay_class={
  .name="sdelay",
  .descr="Signal delay",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
