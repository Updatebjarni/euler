#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"slew.spec.c"

typedef struct slew_module{
  MODULE_BASE
  int value;
  }slew_module;

static int min(int a, int b){
  return (a < b) ? a : b;
}

static void tick(module *_m, int elapsed){
  slew_module *m=(slew_module *)_m;

  int32_t signal=m->input.signal.value;
  int32_t rate=m->input.rate.value;

  if (rate < 0) rate=0;
    
  if (m->value > signal)
    m->value-= min(m->value - signal, rate);

  if (m->value < signal)
    m->value+= min(signal - m->value, rate);

  m->output.value=m->value;
  set_output(&m->output);
  }

class slew_class;

static module *create(char **argv){
  slew_module *m=malloc(sizeof(slew_module));
  base_module_init(m, &slew_class);
  m->value=0;
  return (module *)m;
  }

class slew_class={
  .name="slew",
  .descr="Portamento effect",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
