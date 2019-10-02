#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"poly.spec.c"
#define POLY_CHANNELS (8)

typedef struct poly_module{
  MODULE_BASE
  int time; // Time since start
  int32_t divisor[POLY_CHANNELS];
  int32_t offset[POLY_CHANNELS];
  }poly_module;

static void tick(module *_m, int elapsed){
  poly_module *m=(poly_module *)_m;

  int ticksperloop=m->input.ticksperloop.value;
  int curtime=m->time%ticksperloop;
  
  for (int i=0; i<POLY_CHANNELS; i++){
    //int v=ticksperloop/m->divisor[i];
    int slen=ticksperloop/m->divisor[i];
    int b=curtime%slen;
    if (b<50)
      m->output.gates[i].gate.value=1;    
    else
      m->output.gates[i].gate.value=0;    
    set_output(&m->output.gates[i].gate);
  }

  m->time+=elapsed;
}

static void config(module *_m, char **argv){
  poly_module *m=(poly_module *)_m;

  if (strcmp(argv[0], "divisor")==0){
    for (int i = 0; i<POLY_CHANNELS; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->divisor[i] = to;
    }
  }

  if (strcmp(argv[0], "offsets")==0){
    for (int i = 0; i<POLY_CHANNELS; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->offset[i] = to;
    }
  }
}

class poly_class;

static module *create(char **argv){
  poly_module *m;
  m=malloc(sizeof(poly_module));
 
  base_module_init(m, &poly_class);
  m->time=0;
  for (int i=0; i<POLY_CHANNELS; i++){
    m->divisor[i]=1;
    m->offset[i]=0;
  }

  return (module *)m;
}

class poly_class={
  .name="poly",
  .descr="A polyrythmic gate generator",
  .tick=tick, .destroy=0, .config=config,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
