#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"cquencer.spec.c"

typedef struct cquencer_module{
  MODULE_BASE
  int time; // Time since start
  }cquencer_module;

static void pstep(int input, cquencer_module *m){
  int32_t p = m->input.step[input].pitch.value;
  m->output.pitch.value=p;
}

static void v1step(int input, cquencer_module *m){
  int32_t p = m->input.step[input].value1.value;
  m->output.value1.value=p;
}
  
static void v2step(int input, cquencer_module *m){
  int32_t p = m->input.step[input].value2.value;
  m->output.value2.value=p;
}

static void tick(module *_m, int elapsed){
  cquencer_module *m=(cquencer_module *)_m;
  int32_t ticksperbeat=m->input.ticksperbeat.value;
    
  int pwl=m->time%(8*ticksperbeat);
  int step=pwl/ticksperbeat;
  pwl%=ticksperbeat;

  int gate=0;
  if (pwl<100)
    gate=1;
    
  //m->output.pitch.value=m->pitch[step];
  pstep(step, m);
  v1step(step, m);
  v2step(step, m);
  m->output.gate.value=gate;

  m->time+=elapsed;
}

class cquencer_class;

static module *create(char **argv){
  cquencer_module *m;
  m=malloc(sizeof(cquencer_module));
  base_module_init(m, &cquencer_class);
  m->time=0;
  
  return (module *)m;
  }

class cquencer_class={
  .name="cquencer",
  .descr="A simple 8 step sequencer",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
