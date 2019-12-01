#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"adsr.spec.c"

typedef enum{
  IDLE, ATTACK, DECAY, SUSTAIN, RELEASE
  }adsr_state;

#define LOGLEN 16

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
  int log[LOGLEN];
  int loghead;
  }adsr_module;

static inline int32_t convert_out(adsr_module *m, double a) {
  // linear interpolation
  a=fmax(fmin(1.0, a), 0.0);
  return m->min*(1.0-a)+m->max*a;
  }

// Reads all the inputs
static void readinputs(adsr_module *m) {
  m->attack=m->input.attack.value;
  m->decay=m->input.decay.value;
  m->sustain=m->input.sustain.value;
  m->release=m->input.release.value;
  m->min=m->input.min.value;
  m->max=m->input.max.value;
  }

static void log_append(adsr_module *m, int state){
  if(m->loghead>=LOGLEN)return;
  if(state==-1){
    if(m->loghead && m->log[m->loghead-1]!=-1)
      m->log[m->loghead++]=-1;
    }
  else
    m->log[m->loghead++]=state;
  }

static void next_state(adsr_module *m, int state){
  m->state=state;
  log_append(m, state);
  }

static void tick(module *_m, int elapsed) {
  adsr_module *m=(adsr_module *)_m;
  log_append(m, -1);
  readinputs(m);
    
  double arate=0;
  if (m->attack>0)
    arate=1.0/m->attack;

  double suslevel=0;
  if (m->sustain <= m->max && m->sustain >= m->min){
    double delta=e_max(m->max, m->min) - e_min(m->max, m->min);
    double susdist=m->sustain - e_min(m->max, m->min); 
    if (delta != 0)
      suslevel = susdist/delta;
  }

  double drate=(1.0-suslevel)/m->decay;
  if (m->decay>0)
    drate=(1.0-suslevel)/m->decay;      
    
  double rrate=0;
  if (m->release>0)
    rrate=suslevel/m->release;
    
  switch(m->state){
    case IDLE:
      if (m->input.gate.value) {
        next_state(m, ATTACK);
        m->time=0;
        }
      m->currentamp=0.0;
      break;
    case ATTACK:
      if (m->time>=m->attack || m->attack==0) {
        next_state(m, DECAY);
        m->time=0;
        }
      if (!m->input.gate.value) {
        next_state(m, RELEASE);
        m->time=0;
        }
      m->currentamp=arate*m->time;
      break;
    case DECAY:
      if (!m->input.gate.value) {
        next_state(m, RELEASE);
        m->time=0;
        break;
        }
      if (m->time>=m->decay || m->decay==0) {
        next_state(m, SUSTAIN);
        m->time=0;
        }
      m->currentamp=1.0-m->time*drate;
      break;
    case SUSTAIN:
      if (!m->input.gate.value) {
        next_state(m, RELEASE);
        m->time=0;
        }
      m->currentamp=suslevel;
      break;
    case RELEASE:
      if (m->currentamp<=0 || m->release==0) {
        next_state(m, IDLE);
        m->currentamp=0;
        m->time=0;
        }
      if (m->input.gate.value) {
	next_state(m, ATTACK);
	m->time=0;
      }
      m->currentamp-=rrate;
      break;
    }
    
  m->output.value=convert_out(m, m->currentamp);
  set_output(&m->output);

  if (m->state!=IDLE)
    m->time+=elapsed;
  }

static char *statename[]={"(time passes...)",
                          "IDLE", "ATTACK", "DECAY", "SUSTAIN", "RELEASE"};

static void debug(module *_m){
  adsr_module *m=(adsr_module *)_m;
  for(int i=0; i<m->loghead; ++i)
    printf("%s\n", statename[m->log[i]+1]);
  m->loghead=0;
  }

class adsr_class;

static module *create(char **argv) {
  adsr_module *m;

  m=malloc(sizeof(adsr_module));
  base_module_init(m, &adsr_class);

  m->input.attack.default_value = 0;
  m->input.decay.default_value = 0;
  m->input.sustain.default_value = 1.0;
  m->input.release.default_value = 0;
  m->input.min.default_value = 0;
  m->input.max.default_value = CVMAX;

  m->time=0;
  m->currentamp=0;
  m->state=IDLE;

  m->loghead=0;

  return (module *)m;
  }

class adsr_class={
  .name="adsr",
  .descr="Control signal envelope",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create,
  .debug=debug
  };
