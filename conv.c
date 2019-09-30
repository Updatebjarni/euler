#include<math.h>
#include<stdint.h>
#include"orgel.h"
#include<stdlib.h>

#include"conv.spec.c"

typedef struct conv_module{
  MODULE_BASE
  }conv_module;

static void tick(module *_m, int elapsed){
  conv_module *m=(conv_module *)_m;
  int32_t val=0;

  if (m->input.gate.connection)
    val=m->input.gate.value;
  if (m->input.number.connection)
    val=m->input.number.value;
  if (m->input.virtual_cv.connection)
    val=m->input.virtual_cv.value;

  m->output.gate.value=val;
  m->output.number.value=val;
  m->output.virtual_cv.value=val;
  set_output(&m->output.gate);
  set_output(&m->output.number);
  set_output(&m->output.virtual_cv);
  }

class conv_class;

static module *create(char **argv){
  conv_module *m=malloc(sizeof(conv_module));
  base_module_init(m, &conv_class);
  return (module *)m;
  }

class conv_class={
  .name="conv",
  .descr="Converts from one type of signal to another",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
