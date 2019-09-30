#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"cquencer.spec.c"

typedef enum {
  AUTOMATIC, TIMER
}cquencer_state;

typedef struct cquencer_module{
  MODULE_BASE
  cquencer_state state;
  int steps;
  int time; // Time since start
  int32_t *pitches;
  int32_t **data;
  int32_t *length;
  }cquencer_module;

static void tick(module *_m, int elapsed){
  cquencer_module *m=(cquencer_module *)_m;
  int32_t ticksperbeat=m->input.ticksperbeat.value;
  
  int pwl;
  if (m->state=AUTOMATIC)
    pwl=m->time%(m->steps*ticksperbeat);
  else {
    int t=m->input.time.value;
    pwl=t%(m->steps*ticksperbeat);
  }
    
  int step=pwl/ticksperbeat;
  pwl%=ticksperbeat;

  int gate=0;
  if (pwl<m->length[step])
    gate=1;

  m->output.pitch.value=m->pitches[step];
  m->output.value1.value=m->data[0][step];
  m->output.value2.value=m->data[1][step];
  set_output(&m->output.pitch);
  set_output(&m->output.value1);
  set_output(&m->output.value2);
  m->output.gate.value=gate;
  set_output(&m->output.gate);

  m->time+=elapsed;
}

static void config(module *_m, char **argv){
  cquencer_module *m=(cquencer_module *)_m;

  if (strcmp(argv[0], "pitches")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->pitches[i] = to;
    }
  }

  if (strcmp(argv[0], "length")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->length[i] = to;
    }
  }

  if (strcmp(argv[0], "data")==0){
    long to;
    long dch;
    strtocv(argv[1], &dch);
    for (int i = 0; i<m->steps; i++){
      strtocv(argv[i+2], &to);
      m->data[dch][i] = to;
    }
  }

  if (strcmp(argv[0], "TIMER")==0){
    m->state=TIMER;
  }
}

class cquencer_class;

static module *create(char **argv){
  cquencer_module *m;
  m=malloc(sizeof(cquencer_module));
 
  if (argv) {
    if (strcmp(argv[0], "steps")==0){
      long to;
      strtocv(argv[1], &to);
      m->steps = to;
    }
  } else {
    fprintf(stderr, "No size specified. Using default = 8 steps\n");
    m->steps = 8;
  }
  
  m->data = malloc(sizeof(int32_t*)*2);
  m->data[0] = malloc(sizeof(int32_t)*m->steps);
  m->data[1] = malloc(sizeof(int32_t)*m->steps);
  m->pitches = malloc(sizeof(int32_t)*m->steps);
  m->length = malloc(sizeof(int32_t)*m->steps);
  
  base_module_init(m, &cquencer_class);
  m->time=0;
  m->state=AUTOMATIC;
  for (int i=0; i<m->steps; i++){
    m->pitches[i]=0;
    m->data[0][i]=0;
    m->data[1][i]=0;
    m->length[i]=0;
  }

  return (module *)m;
}

class cquencer_class={
  .name="cquencer",
  .descr="A simple N step sequencer",
  .tick=tick, .destroy=0, .config=config,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
