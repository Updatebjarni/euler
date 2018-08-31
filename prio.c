#include<stdlib.h>
#include"orgel.h"

#include"prio.spec.c"

enum{LEFT_PRIO, RIGHT_PRIO, LAST_PRIO, FIRST_PRIO};
enum{ALLOC_NEXT, ALLOC_RR};

typedef struct prio_module{
  module;
  int prio, alloc;
  int held[10], nheld;
  int polyphony;
  }prio_module;

static void tick(module *_m, int elapsed){
  prio_module *m=(prio_module *)_m;
  if(m->input.in_terminal.connection){
    jack_value *v=&(m->input.in_terminal.connection->out_terminal.value);
    for(int i=0; i<v->key_events.len; ++i){
      if(v->key_events.buf[i].state==KEY_DOWN){
        m->held[m->nheld]=v->key_events.buf[i].key;
        ++(m->nheld);
        }
      else{
        int j=0;
        for(int k=0; k<m->nheld; ++k)
          if(v->key_events.buf[i].key!=m->held[k]){
            m->held[j]=m->held[k];
            ++j;
            }
        m->nheld=j;
        }
      }
    ((struct output_bundle *)m->output.bundle.elements)->monophone[0].bundle-\
>_pitch.element.out_terminal.changed=1;
    ((struct output_bundle *)m->output.bundle.elements)->monophone[0].bundle-\
>_gate.element.out_terminal.changed=1;
    if(m->nheld){
      ((struct output_bundle *)m->output.bundle.elements)->monophone[0].bundle->pitch.int32=
        m->held[0]*HALFNOTE;
      ((struct output_bundle *)m->output.bundle.elements)->monophone[0].bundle->gate.bool=1;
      }
    else ((struct output_bundle *)m->output.bundle.elements)->monophone[0].bundle->gate.bool=0;
    }
  }

static void config(module *m, char **argv){
  int n;
  if(!argv[0]){
    printf("Use 'config prio notes=n' to configure for n notes.\n");
    return;
    }
  if(sscanf(argv[0], "notes=%d", &n)!=1){
    printf("Unknown parameter \"%s\".\n", argv[0]);
    return;
    }
  LOCK_MODULES();
  UNLOCK_MODULES();
  }

class prio_class;

static module *create(char **argv){
  prio_module *m;

  m=malloc(sizeof(prio_module));
  default_module_init((module *)m, &prio_class);
  m->prio=LEFT_PRIO;
  m->alloc=ALLOC_RR;
  m->polyphony=1;
  m->nheld=0;

  return (module *)m;
  }

class prio_class={
  "prio", "Note priority",
  &input, &output,
  tick, 0, config,
  DYNAMIC_CLASS,
  create,
  0
  };
