#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"cquencer.spec.c"

typedef struct cquencer_module{
  MODULE_BASE
  int steps;
  int time; // Time since start
  int32_t *pitches;
  int32_t **data;
  int32_t *length;
  int offset;
  }cquencer_module;

static void tick(module *_m, int elapsed){
  cquencer_module *m=(cquencer_module *)_m;
  int32_t ticksperbeat=m->input.ticksperbeat.value;
  
  int pwl;
  if (!m->input.time.connection)
    pwl=(m->time+m->offset)%(m->steps*ticksperbeat);
  else {
    // If an input is connected to time jack,
    // use the value on the input to compute
    // position in loop.
    int t=(m->input.time.value+m->offset);
    pwl=t%(m->steps*ticksperbeat);
  }
    
  int step=pwl/ticksperbeat;
  pwl%=ticksperbeat;

  int gate=0;
  if (m->input.gate.connection){
    if (m->input.gate.value && pwl<m->length[step])
      gate=1;
  } else {
    if (pwl<m->length[step])
      gate=1;
  }

  m->output.pitch.value=m->pitches[step];
  m->output.data[0].value.value=m->data[0][step];
  m->output.data[1].value.value=m->data[1][step];
  m->output.data[2].value.value=m->data[2][step];
  m->output.data[3].value.value=m->data[3][step];
  set_output(&m->output.pitch);
  set_output(&m->output.data[0].value);
  set_output(&m->output.data[1].value);
  set_output(&m->output.data[2].value);
  set_output(&m->output.data[3].value);
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
    if (dch<4){
      for (int i = 0; i<m->steps; i++){
	strtocv(argv[i+2], &to);
	m->data[dch][i] = to;
      }
    } else {
      printf("ERROR: Out of bounds insertion in cquencer data: %ld \n",dch);
    }
  }

  if (strcmp(argv[0], "offset")==0){
    long t;
    strtocv(argv[1], &t);
    m->offset=t;
  }
}

class cquencer_class;

static void destroy(module *m){
  cquencer_module *_m = (cquencer_module*)m;
  free(_m->data[0]);
  free(_m->data[1]);
  free(_m->data[2]);
  free(_m->data[3]);
  free(_m->pitches);
  free(_m->length);
  free(_m->data);
}

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
  
  m->data = malloc(sizeof(int32_t*)*4);
  m->data[0] = malloc(sizeof(int32_t)*m->steps);
  m->data[1] = malloc(sizeof(int32_t)*m->steps);
  m->data[2] = malloc(sizeof(int32_t)*m->steps);
  m->data[3] = malloc(sizeof(int32_t)*m->steps);
  m->pitches = malloc(sizeof(int32_t)*m->steps);
  m->length = malloc(sizeof(int32_t)*m->steps);
  
  base_module_init(m, &cquencer_class);
  m->time=0;
  m->offset=0;
  for (int i=0; i<m->steps; i++){
    m->pitches[i]=0;
    m->data[0][i]=0;
    m->data[1][i]=0;
    m->data[2][i]=0;
    m->data[3][i]=0;
    m->length[i]=0;
  }

  return (module *)m;
}

class cquencer_class={
  .name="cquencer",
  .descr="A simple N step sequencer",
  .tick=tick, .destroy=destroy, .config=config,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
