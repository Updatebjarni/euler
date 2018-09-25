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
  }adsr_module;

static inline int32_t convert_out(adsr_module *m, double a) {
  int32_t max=CVMAX;
  int32_t min=0;
  if (INPUT(m)->min.connection)
    min=INPUT(m)->min.connection->value;
  if (INPUT(m)->max.connection)
    max=INPUT(m)->max.connection->value;
  // linear interpolation
  return min*(1.0-a)+max*a;
  }

static void tick(module *_m, int elapsed) {
  adsr_module *m=(adsr_module *)_m;
  if (INPUT(m)->gate.connection &&
      INPUT(m)->attack.connection &&
      INPUT(m)->decay.connection &&
      INPUT(m)->sustain.connection &&
      INPUT(m)->release.connection) {
 
    int32_t attack=(INPUT(m)->attack.connection->value);
    int32_t decay=(INPUT(m)->decay.connection->value);
    int32_t sustain=(INPUT(m)->sustain.connection->value);
    int32_t release=(INPUT(m)->release.connection->value);

    double convsus=((double)sustain)/CVMAX;
    double arate=1.0/attack;
    double drate=(1.0-convsus)/decay;
    double rrate=convsus/release;
    
    switch(m->state) {
      case IDLE:
	if (INPUT(m)->gate.connection->value){
	  m->state=ATTACK;
	  m->time=0; // reset timer
	  }
	m->currentamp=0.0;
	break;
      case ATTACK:
	if (m->time>=attack) {
	  m->state=DECAY;
	  m->time=0;
    	  }
	if (!INPUT(m)->gate.connection->value){
	  m->state=RELEASE;
	  m->time=0;
	  }
	m->currentamp=arate*m->time;
        break;
      case DECAY:
	if (!INPUT(m)->gate.connection->value){
	  m->state=RELEASE;
	  m->time=0;
  	  }
	if (m->time>=decay) {
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
	m->currentamp=convsus;
        break;
      case RELEASE:
	m->currentamp-=rrate;
	if (m->currentamp<=0) {
	  m->currentamp=0;
	  m->state=IDLE;
   	  }
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
