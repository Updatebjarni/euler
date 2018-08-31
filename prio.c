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
struct{int note, voice;}playing[3];
  }prio_module;

static void tick(module *_m, int elapsed){
  prio_module *m=(prio_module *)_m;
  if(m->input.in_terminal.connection){
    jack_value *v=&(m->input.in_terminal.connection->out_terminal.value);
    for(int i=0; i<v->key_events.len; ++i){
      if(v->key_events.buf[i].state==KEY_DOWN){
int voice=m->playing[0].voice;
m->playing[0]=m->playing[1];
m->playing[1]=m->playing[2];
m->playing[2].voice=voice;
m->playing[2].note=v->key_events.buf[i].key;
((struct output_bundle *)m->output.bundle.elements)->monophone[voice].bundle
                          ->pitch.int32=v->key_events.buf[i].key*HALFNOTE;
((struct output_bundle *)m->output.bundle.elements)->monophone[voice].bundle
                          ->gate.bool=1;

       m->held[m->nheld]=v->key_events.buf[i].key;
        ++(m->nheld);
        }
      else{
for(int n=0; n<3; ++n){
  if(m->playing[n].note==v->key_events.buf[i].key){
    int voice=m->playing[n].voice;
    for(int x=n; x>0; --x)m->playing[x]=m->playing[x-1];
    m->playing[0].voice=voice; m->playing[0].note=-1;
    ((struct output_bundle *)m->output.bundle.elements)->monophone[voice].bundle
                              ->gate.bool=0;
    break;
    }
  }
        int j=0;
        for(int k=0; k<m->nheld; ++k)
          if(v->key_events.buf[i].key!=m->held[k]){
            m->held[j]=m->held[k];
            ++j;
            }
        m->nheld=j;
        }
      }
for(int n=0; n<3; ++n){
    ((struct output_bundle *)m->output.bundle.elements)->monophone[n].bundle
                              ->_pitch.element.out_terminal.changed=1;
    ((struct output_bundle *)m->output.bundle.elements)->monophone[n].bundle
                              ->_gate.element.out_terminal.changed=1;
  }
    if(m->nheld){

      }
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
for(int i=0; i<3; ++i){m->playing[i].note=-1; m->playing[i].voice=i;}
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
