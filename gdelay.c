#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"gdelay.spec.c"

typedef struct gdelay_module{
  MODULE_BASE
  int maxticks;  // Number of ticks delay
  int *gate;
  }gdelay_module;

static void tick(module *_m, int elapsed){
  gdelay_module *m=(gdelay_module *)_m;
  int32_t gate=m->input.gate.value;
  int32_t delay=m->input.delay.value;

  if (delay > m->maxticks)
    delay = m->maxticks;

  if (delay < 0)
    delay = 0;
    
  for (int i=delay; i>0; i--){
    m->gate[i]=m->gate[i-1];
    }

  m->gate[0]=gate;
    
  m->output.value=m->gate[delay];
  set_output(&m->output);
  }

class gdelay_class;

static module *create(char **argv){
  gdelay_module *m;

  m=malloc(sizeof(gdelay_module));
  base_module_init(m, &gdelay_class);
  m->maxticks=10000;
  m->gate=malloc(sizeof(int32_t)*m->maxticks);
  for (int i = 0; i<1000*2; i++){
    m->gate[i]=0;
    }
  return (module *)m;
  }

class gdelay_class={
  .name="gdelay",
  .descr="Gate delay",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create,
  };
