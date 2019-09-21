#include<math.h>
#include<stdint.h>
#include"orgel.h"
#include<stdlib.h>

#include"sah.spec.c"

typedef struct sah_module{
  MODULE_BASE
  int time;
  int32_t value;
  }sah_module;

static void tick(module *_m, int elapsed){
  sah_module *m=(sah_module *)_m;
  
  if (m->time >= m->input.time.value){
    m->time = 0;
  }

  if (m->time == 0){
    m->value=m->input.signal.value;
  }

  m->time++;
  m->output.value=m->value;
  set_output(&m->output);
  }

class sah_class;

static module *create(char **argv){
  sah_module *m=malloc(sizeof(sah_module));
  base_module_init(m, &sah_class);
  m->time=0;
  m->value=0;
  return (module *)m;
  }

class sah_class={
  .name="sah",
  .descr="Sample and hold",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
