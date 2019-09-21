#include<math.h>
#include<stdint.h>
#include"orgel.h"
#include<stdlib.h>
#include<stdio.h>

#include"random.spec.c"

typedef struct random_module{
  MODULE_BASE
  int time;
  int32_t value;
  }random_module;

static void tick(module *_m, int elapsed){
  random_module *m=(random_module *)_m;
  
  if (m->time >= m->input.time.value){
    m->time = 0;
  }

  if (m->time == 0){
    int32_t upper = m->input.upper.value;
    int32_t lower = m->input.lower.value;
    m->value=rand()%(upper-lower+1)+lower;
  }

  m->time++;
  m->output.value=m->value;
  set_output(&m->output);
  }

class random_class;

static module *create(char **argv){
  random_module *m=malloc(sizeof(random_module));
  base_module_init(m, &random_class);
  m->time=0;
  m->value=0;
  return (module *)m;
  }

class random_class={
  .name="random",
  .descr="Generates random values",
  .create=create,
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  };
