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
  module;
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
  if (INPUT(m)->attack.connection)
    m->attack=(INPUT(m)->attack.connection->value);
  m->decay=0;
  if (INPUT(m)->decay.connection)
    m->decay=(INPUT(m)->decay.connection->value);
  m->sustain=1.0;
  if (INPUT(m)->sustain.connection)
    m->sustain=(INPUT(m)->sustain.connection->value);
  m->release=0;
  if (INPUT(m)->release.connection)
    m->release=(INPUT(m)->release.connection->value);
  m->min=0;
  if (INPUT(m)->min.connection)
    m->min=INPUT(m)->min.connection->value;
  m->max=CVMAX;
  if (INPUT(m)->max.connection)
    m->max=INPUT(m)->max.connection->value;
  }

static void tick(module *_m, int elapsed) {
  adsr_module *m=(adsr_module *)_m;
  if (INPUT(m)->gate.connection) {

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
    
    switch(m->state)
      {
      case IDLE:
	if (INPUT(m)->gate.connection->value) {
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
	if (!INPUT(m)->gate.connection->value) {
	  m->state=RELEASE;
	  m->time=0;
	  }
	m->currentamp=arate*m->time;
        break;
      case DECAY:
	if (!INPUT(m)->gate.connection->value) {
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
	if (!INPUT(m)->gate.connection->value) {
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
    
    OUTPUT(m).int32_value=convert_out(m, m->currentamp);
    
    if (m->state!=IDLE)
      m->time+=elapsed;
    }
  }

class adsr_class;

static module *create(char **argv) {
  adsr_module *m;

  m=malloc(sizeof(adsr_module));
  default_module_init((module *)m, &adsr_class);
  m->time=0;
  m->currentamp=0;
  m->state=IDLE;
  return (module *)m;
  }

class adsr_class={
  "adsr",                    // char *name
  "Control signal envelope", // char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };
