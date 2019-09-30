#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"slew.spec.c"

typedef enum{
  LIN, LOG
  }slew_mode ;

typedef struct slew_module{
  MODULE_BASE
  slew_mode mode;
  int32_t value;
  int32_t delta;
  int32_t lastvalin;
  int32_t rate; // For use in div
  }slew_module;

static int min(int a, int b){
  return (a < b) ? a : b;
}

void ticklin(slew_module *m){
  int32_t signal=m->input.signal.value;
  int32_t rate=m->input.rate.value;

  if (m->value > signal)
    m->value-= min(m->value - signal, rate);
  if (m->value < signal)
    m->value+= min(signal - m->value, rate);
  m->output.value=m->value;
  set_output(&m->output);
}

void ticklog(slew_module *m){
  int32_t signal=m->input.signal.value;
  int32_t rate=m->input.rate.value;
  double coeff=((double)rate)/((double)CVMAX);
  coeff = fmax(fmin(coeff, 1.0), 0.0);

  if (m->value!=signal){
    m->delta=m->value-signal;
    m->value=signal;
  }

  m->delta=(int32_t)(((double)m->delta)*coeff);
  m->output.value=m->value+m->delta;
  set_output(&m->output);
}

static void tick(module *_m, int elapsed){
  slew_module *m=(slew_module *)_m;
  switch(m->mode){
  case LIN:
    ticklin(m);
    break;
  case LOG:
  default:
    ticklog(m);
    break;
  }
}

class slew_class;

static module *create(char **argv){
  slew_module *m=malloc(sizeof(slew_module));
  base_module_init(m, &slew_class);
  m->value=0;
  m->mode=LOG;
  m->lastvalin=0;
  m->rate=0;
  return (module *)m;
  }

class slew_class={
  .name="slew",
  .descr="Linear and logarithmic slew limiter",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
