#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"arpp.spec.c"

typedef struct arpp_module{
  MODULE_BASE
  int steps;
  int time; // Time since start
  int32_t *num;
  int32_t *on;
  int32_t *tie;
  int32_t *ntransp;
  int32_t *otransp;
  int32_t held[256];
  }arpp_module;

static int keyfind(int n, int32_t *heldk){
  int counter=0;
  for(int i=0; i<256; ++i){
    if (heldk[i]){
      if (counter==n)
	return i;
      counter++;
    }
  }
  return -1;
}

static void tick(module *_m, int elapsed){
  arpp_module *m=(arpp_module *)_m;
  int32_t ticksperbeat=m->input.ticksperbeat.value;

  key_events *v=&(m->input.keys.value);
  for(int i=0; i<v->len; ++i){
    int key=v->buf[i].key;
    if(v->buf[i].state==KEY_DOWN){
      m->held[key]=1;
    } else {
      m->held[key]=0;
    }
  }

  int pwl=(m->time)%(m->steps*ticksperbeat);
  int step=pwl/ticksperbeat;
  pwl%=ticksperbeat;

  int32_t k=keyfind(m->num[step], m->held);
  if (k>=0)
    m->output.pitch.value=(k+m->ntransp[step]+12*m->otransp[step])*HALFNOTE;

  int gate=0;
  if(k>=0){
    if (pwl<80 && m->on[step]){
      m->output.gate.value=1;
    } else {
      m->output.gate.value=0;
    }
  }

  set_output(&m->output.gate);
  set_output(&m->output.pitch);

  m->time+=elapsed;
}

static void config(module *_m, char **argv){
  arpp_module *m=(arpp_module *)_m;

  if (strcmp(argv[0], "on")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->on[i] = to;
    }
  }

  if (strcmp(argv[0], "num")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->num[i] = to;
    }
  }

  if (strcmp(argv[0], "ntransp")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->ntransp[i] = to;
    }
  }

  if (strcmp(argv[0], "otransp")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->otransp[i] = to;
    }
  }


}

class arpp_class;

static void destroy(module *m){
  arpp_module *_m = (arpp_module*)m;
  free(_m->on);
  free(_m->tie);
  free(_m->num);
  free(_m->ntransp);
  free(_m->otransp);
}

static module *create(char **argv){
  arpp_module *m;
  m=malloc(sizeof(arpp_module));
 
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
  
  base_module_init(m, &arpp_class);
  m->time=0;
  for (int i=0; i<256; i++)
    m->held[i]=0;

  m->on=malloc(sizeof(int32_t)*m->steps);
  m->tie=malloc(sizeof(int32_t)*m->steps);
  m->num=malloc(sizeof(int32_t)*m->steps);
  m->ntransp=malloc(sizeof(int32_t)*m->steps);
  m->otransp=malloc(sizeof(int32_t)*m->steps);


  for (int i=0; i<m->steps; ++i){
    m->on[i]=0;
    m->tie[i]=0;
    m->num[i]=0;
    m->ntransp[i]=0;
    m->otransp[i]=0;
  }

  return (module *)m;
}

class arpp_class={
  .name="arpp",
  .descr="Arpeggiator inspired by the FM8 Arpeggiator",
  .tick=tick, .destroy=destroy, .config=config,
  .is_static=DYNAMIC_CLASS,
  .create=create
  };
