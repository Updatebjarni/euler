#include<stdlib.h>
#include<math.h>
#include<stdint.h>
#include"orgel.h"

#include"na.spec.c"


typedef struct na_module{
  MODULE_BASE
  }na_module;

static void tick(module *_m, int elapsed){
  na_module *m=(na_module *)_m;
  int64_t result=(int64_t)(m->input.signal.value);
  result*=pow(2, 1.0*m->input.control.value/OCTAVE);
  if(result>INT32_MAX)result=INT32_MAX;
  if(result<INT32_MIN)result=INT32_MIN;
  m->output.value=result;
  set_output(&m->output);
  }

class na_class;

static module *create(char **argv){
  na_module *m=malloc(sizeof(na_module));
  base_module_init(m, &na_class);
  return (module *)m;
  }

class na_class={
  .name="na",
  .descr="Numerical attenuverter",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
