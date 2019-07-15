#include<math.h>
#include<stdint.h>
#include"orgel.h"

#include"add.spec.c"


typedef struct add_module{
  MODULE_BASE
  }add_module;

static void tick(module *_m, int elapsed){
  add_module *m=(add_module *)_m;
  int64_t result=(int64_t)(m->input.signal.value);
  result+=m->input.control.value;
  if(result>INT32_MAX)result=INT32_MAX;
  if(result<INT32_MIN)result=INT32_MIN;
  m->output.value=result;
  }

class add_class;

static module *create(char **argv){
  add_module *m=malloc(sizeof(add_module));
  base_module_init(m, &add_class);
  return (module *)m;
  }

class add_class={
  .name="add",
  .descr="Add two signals",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
