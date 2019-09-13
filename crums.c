#include<stdlib.h>
#include"orgel.h"
#include"orgel-io.h"

#include"crums.spec.c"


typedef struct crums_module{
  MODULE_BASE
  }crums_module;

static char trig_d, trig_b, trig_cl, trig_l, trig_br, trig_sd;

static void tick(module *_m, int elapsed){
  crums_module *m=(crums_module *)_m;
  SELECT_MODULE(0);
  unsigned char byte=0;

  if(m->input.d.valchanged && m->input.d.value){
    trig_d=10;
    m->input.d.valchanged=0;
    }
  if(m->input.b.valchanged && m->input.b.value){
    trig_b=10;
    m->input.b.valchanged=0;
    }
  if(m->input.cl.valchanged && m->input.cl.value){
    trig_cl=10;
    m->input.cl.valchanged=0;
    }
  if(m->input.l.valchanged && m->input.l.value){
    trig_l=10;
    m->input.l.valchanged=0;
    }
  if(m->input.br.valchanged && m->input.br.value){
    trig_br=10;
    m->input.br.valchanged=0;
    }
  if(m->input.sd.valchanged && m->input.sd.value){
    trig_sd=10;
    m->input.sd.valchanged=0;
    }

  byte=((!!trig_d)<<0)|((!!trig_b)<<1)|((!!trig_cl)<<2)|
       ((!!trig_l)<<3)|((!!trig_br)<<4)|((!!trig_sd)<<5);
  MODULE_WRITE(byte);

  if(trig_d)--trig_d;
  if(trig_b)--trig_b;
  if(trig_cl)--trig_cl;
  if(trig_l)--trig_l;
  if(trig_br)--trig_br;
  if(trig_sd)--trig_sd;
  }

static void reset(module *m){
  LOCK_NEST();
  SELECT_MODULE(0);
  MODULE_SET_REG(1);
  MODULE_WRITE(0);
  LOCK_UNNEST();
  }

class crums_class;

static module *create(char **argv){
  crums_module *m=malloc(sizeof(crums_module));
  base_module_init(m, &crums_class);
  reset((module *)m);
  return (module *)m;
  }

class crums_class={
  .name="crums", .descr="Crumar Drums",
  .tick=tick, .destroy= 0, .config=0,
  .is_static=STATIC_CLASS,
  .create=create,
  .create_counter=0,
  .reset=reset
  };
