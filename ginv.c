#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include"orgel.h"
#include<stdlib.h>

#include"ginv.spec.c"

typedef struct ginv_module{
  MODULE_BASE
  }ginv_module;

static void tick(module *_m, int elapsed){
  ginv_module *m=(ginv_module *)_m;
  m->output.value=!m->input.gate.value;
  set_output(&m->output);
  }

class ginv_class;

static module *create(char **argv){
  ginv_module *m=malloc(sizeof(ginv_module));
  base_module_init(m, &ginv_class);
  return (module *)m;
  }

class ginv_class={
  .name="ginv",
  .descr="Gate inverter",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
