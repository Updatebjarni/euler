#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"pulse.spec.c"

typedef struct pulse_module {
  MODULE_BASE;
  int time;
  int on;
  }pulse_module;

static void tick(module *_m, int elapsed) {
  pulse_module *m=(pulse_module *)_m;
  
  if(m->input.gate.value) {
    if(m->time==0 && (!m->on)) {
      m->on=1;
      //m->time=INPUT(m)->length.connection->value;
      m->time=m->input.length.value;
    }
  } else {
    if(m->on)
      m->on=0;
  }
  
  if(m->time>0) {
    if(m->input.height.connection) {
      //OUTPUT(m).int32_value=INPUT(m)->height.connection->value;
      m->output.value=m->input.height.value;
    }
    else { 
      m->output.value=CVMAX;
      //OUTPUT(m).int32_value=CVMAX;
    }
    m->time--;
  } else {
    m->output.value=0;
    //OUTPUT(m).int32_value=0;
  }

  set_output(&m->output);
}

class pulse_class;

static module *create(char **argv) {
  pulse_module *m;
  m=malloc(sizeof(pulse_module));
  base_module_init(m, &pulse_class);
  m->time=0;
  m->on=0;
  return (module *)m;
  }

class pulse_class={
  .name="pulse",
  .descr="Generates an output signal while gate is on",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
