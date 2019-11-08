#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include"orgel.h"
#include<stdlib.h>

#include"clamp.spec.c"

typedef struct clamp_module{
  MODULE_BASE
  }clamp_module;

static void tick(module *_m, int elapsed){
  clamp_module *m=(clamp_module *)_m;
  int32_t in=m->input.signal.value;
  int32_t max=m->input.max.value;
  int32_t min=m->input.min.value;
  if(in>max)in=max;
  if(in<min)in=min;
  m->output.value=in;
  set_output(&m->output);
  }

class clamp_class;

static module *create(char **argv){
  clamp_module *m=malloc(sizeof(clamp_module));
  base_module_init(m, &clamp_class);
  return (module *)m;
  }

class clamp_class={
  .name="clamp",
  .descr="Clamps input signal between max and min inputs",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
