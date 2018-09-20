#include<stdlib.h>
#include"orgel.h"

#include"prio.spec.c"

enum{PRIO_LEFT, PRIO_RIGHT, PRIO_FIRST, PRIO_LAST, PRIO_STACK};
enum{ALLOC_FIRST, ALLOC_RR};

typedef struct prio_module{
  module;
  int prio, alloc;
  struct{int key, voice, changed;}held[256];
  int nheld;
  int *voices;
  int nvoices, nplaying;
  }prio_module;
 
static void tick(module *_m, int elapsed){
  prio_module *m=(prio_module *)_m;
  if(m->input.in_terminal.connection){
    jack_value *v=(jack_value *)
      &(INPUT(m).connection->out_terminal.key_events_value);
    for(int i=0; i<v->key_events_value.len; ++i){
      int key=v->key_events_value.buf[i].key;
      if(v->key_events_value.buf[i].state==KEY_DOWN){
        int ins=0;
        switch(m->prio){
          case PRIO_LEFT:
            for(ins=0; ins<m->nheld && key>m->held[ins].key; ++ins);
            break;
          case PRIO_RIGHT:
            for(ins=0; ins<m->nheld && key<m->held[ins].key; ++ins);
          case PRIO_FIRST:
            ins=m->nheld;
            break;
          case PRIO_LAST:
          case PRIO_STACK:
            ins=0;
            break;
          }
        for(int j=m->nheld; j>ins; --j)
          m->held[j]=m->held[j-1];
        m->held[ins].key=key;
        if(ins<m->nvoices){
          if(m->nplaying<m->nvoices){
            int next=m->nvoices-m->nplaying-1;
            m->held[ins].voice=m->voices[next];
            ++(m->nplaying);
            }
          else
            m->held[ins].voice=m->held[m->nvoices].voice;
          m->held[ins].changed=1;
          }
        else
          m->held[ins].voice=-1;
        ++(m->nheld);
        }
      else{
        int del;
        for(del=0; del<m->nheld && key!=m->held[del].key; ++del);
        if(del<m->nheld){
          int freed=m->held[del].voice;
          for(int j=del; j<m->nheld-1; ++j)
            m->held[j]=m->held[j+1];
          --(m->nheld);
          if(del<m->nplaying){
            if((m->nheld >= m->nvoices) && (m->prio!=PRIO_LAST)){
              m->held[m->nvoices-1].voice=freed;
              m->held[m->nvoices-1].changed=1;
              }
            else{
              m->voices[m->nvoices-m->nplaying]=freed; // *** alloc first vs rr
              --(m->nplaying);
              }
            }
          }
        }
      }
    struct output_bundle *out=
      (struct output_bundle *)m->output.bundle;
    for(int i=0; i<m->nplaying && i<m->nheld; ++i){
      if(m->held[i].changed){
        int voice=m->held[i].voice;
        out->monophone.INDEX(voice)->pitch.int32_value=m->held[i].key*HALFNOTE;
        out->monophone.INDEX(voice)->pitch.changed=1;
        out->monophone.INDEX(voice)->gate.bool_value=1;
        out->monophone.INDEX(voice)->gate.changed=1;
        }
      }
    for(int i=0; i<(m->nvoices-m->nplaying); ++i){
      int voice=m->voices[i];
      out->monophone.INDEX(voice)->gate.bool_value=0;
      out->monophone.INDEX(voice)->gate.changed=1;
      }
    }
  }

static paramspec config_params[]={
  {"voices", 1, PARAM_NUMBER, .intval=0},
  {0}
  };

static void config(module *_m, char **argv){
  prio_module *m=(prio_module *)_m;
  int n;
  if(!argv[0]){
    printf("Use 'config prio voices=n' to configure for n voices.\n");
    return;
    }
  if(parse_param(&argv, config_params)!=1){
    printf("Unknown parameter \"%s\".\n", argv[0]);
    return;
    }
  n=config_params[0].intval;
  LOCK_MODULES();
  m->nvoices=n;
  m->voices=realloc(m->voices, sizeof(int)*n);
  for(int i=0; i<n; ++i)
    m->voices[i]=i;
  for(int i=0; i<OUTPUT(m)->monophone.element.len; ++i)
    disconnect_tree((jack *)&(OUTPUT(m)->monophone.INDEX(i)));
  OUTPUT(m)->monophone.element.len=n;
// *** This might move the array and break the pointers pointing
//     here from the inputs:
  OUTPUT(m)->monophone.ptr=
    realloc(OUTPUT(m)->monophone.ptr, sizeof(OUTPUT(m)->monophone.ptr[0])*n);
  for(int i=0; i<n; ++i)
    create_jack((jack *)(OUTPUT(m)->monophone.ptr+i),
                (jack *)(&output_monophone_template), DIR_OUT, m);
  UNLOCK_MODULES();
  }

static void debug(module *_m){
  prio_module *m=(prio_module *)_m;
  printf("Held: [ ");
  for(int i=0; i<m->nheld; ++i)
    printf("k%d,v%d,c%d ",
           m->held[i].key, m->held[i].voice, m->held[i].changed);
  printf("]\nPlaying: %d\n", m->nplaying);
  printf("Voices: ");
  for(int i=0; i<(m->nvoices-m->nplaying); ++i)
    printf("%d ", m->voices[i]);
  printf("\n");
  }

class prio_class;

static module *create(char **argv){
  prio_module *m;

  m=malloc(sizeof(prio_module));
  default_module_init((module *)m, &prio_class);
  m->prio=PRIO_LAST;
  m->alloc=ALLOC_RR;
  m->nheld=0;
  m->nvoices=1;
  m->nplaying=0;
  m->voices=malloc(sizeof(int)*m->nvoices);
  for(int i=0; i<m->nvoices; ++i)
    m->voices[i]=i;
  config((module *)m, argv);
  return (module *)m;
  }

class prio_class={
  "prio", "Note priority",
  &input, &output,
  tick, 0, config,
  DYNAMIC_CLASS,
  create,
  0,
  debug
  };
