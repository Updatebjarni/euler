#include"orgel.h"

#include"prio.spec.c"

static void tick(module *m, int elapsed){
  }

static void config(module *m, char **argv){
  LOCK_MODULES();
  UNLOCK_MODULES();
  }

static int prio_init(module *m, char **argv){
  return 0;
  }

class prio_class={
  "prio", "Note priority",
  &input, &output,
  tick, 0, config,
  DYNAMIC_CLASS,
  prio_init,
  0
  };
