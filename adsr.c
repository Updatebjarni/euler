#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"adsr.spec.c"

typedef enum{
  IDLE, ATTACK, DECAY, SUSTAIN, RELEASE
  }adsr_state;

typedef struct adsr_module{
  MODULE_BASE
  int time;
  double currentamp;
  adsr_state state;
  int32_t max;
  int32_t min;
  int32_t attack;
  int32_t decay;
  int32_t sustain;
  int32_t release;
  }adsr_module;

static inline int32_t convert_out(adsr_module *m, double a) {
  // linear interpolation
  a=fmax(fmin(1.0, a), 0.0);
  return m->min*(1.0-a)+m->max*a;
  }

// Reads all the inputs and assigns default values
static void readinputs(adsr_module *m) {
  m->attack=0;
  m->attack=(m->input.attack.value);
  m->decay=0;
  m->decay=(m->input.decay.value);
  m->sustain=1.0;
  m->sustain=(m->input.sustain.value);
  m->release=0;
  m->release=(m->input.release.value);
  m->min=0;
  m->min=m->input.min.value;
  m->max=CVMAX;
  m->max=m->input.max.value;
  }

static void tick(module *_m, int elapsed) {
  adsr_module *m=(adsr_module *)_m;

  readinputs(m);
    
  double arate=0;
  if (m->attack>0)
    arate=1.0/m->attack;

  double suslevel=0;
  if (m->max>0)
    suslevel=((double)m->sustain)/((double)m->max);

  double drate=(1.0-suslevel)/m->decay;
  if (m->decay>0)
    drate=(1.0-suslevel)/m->decay;      
    
  double rrate=0;
  if (m->release>0)
    rrate=suslevel/m->release;
    
  switch(m->state){
    case IDLE:
      if (m->input.gate.value) {
        m->state=ATTACK;
        m->time=0; // reset timer
        }
      m->currentamp=0.0;
      break;
    case ATTACK:
      if (m->time>=m->attack || m->attack==0) {
        m->state=DECAY;
        m->time=0;
        }
      if (!m->input.gate.value) {
        m->state=RELEASE;
        m->time=0;
        }
      m->currentamp=arate*m->time;
      break;
    case DECAY:
      if (!m->input.gate.value) {
        m->state=RELEASE;
        m->time=0;
        }
      if (m->time>=m->decay || m->decay==0) {
        m->state=SUSTAIN;
        m->time=0;
        }
      m->currentamp=1.0-m->time*drate;
      break;
    case SUSTAIN:
      if (!m->input.gate.value) {
        m->state=RELEASE;
        m->time=0;
        }
      m->currentamp=suslevel;
      break;
    case RELEASE:
      if (m->currentamp<=0 || m->release==0) {
        m->state=IDLE;
        m->currentamp=0;
        m->time=0;
        }
      m->currentamp-=rrate;
      break;
    }
    
  m->output.value=convert_out(m, m->currentamp);

  if (m->state!=IDLE)
    m->time+=elapsed;
  }

class adsr_class;

static module *create(char **argv) {
  adsr_module *m;

  m=malloc(sizeof(adsr_module));
  base_module_init(m, &adsr_class);
  m->time=0;
  m->currentamp=0;
  m->state=IDLE;
  return (module *)m;
  }

class adsr_class={
  .name="adsr",
  .descr="Control signal envelope",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
