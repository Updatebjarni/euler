#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"arpp.spec.c"

typedef enum{
  WRAP, OCTAVE1, IGNORE
}keywrap;

typedef enum{
  HOLD, GLIDE
}holdbehavior;

typedef struct arpp_module{
  MODULE_BASE
  int steps; // Size of sequence
  int time; // Time since start
  int keys; // Number of keys held
  int recent;
  int32_t *num; // Play the given key or random if negative
  int32_t *on;  // Plays this step if true
  int32_t *hold; // TODO
  int32_t *ntransp; // Note transpose
  int32_t *otransp; // Octave transpose
  int32_t held[256];
  }arpp_module;

// Find the n:th key pressed from the left 
// among all held keys. Indexed from zero.
static int keyfind(int n, int numkeys, keywrap mode, int32_t *heldk){
  int counter=0;
  if (numkeys==0)
    return -1;
  switch(mode){
    case WRAP:
      n%=numkeys;
      break;
    default:
      break;
  }
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
  int32_t gatelength=m->input.gatelength.value;
  double dt=(double)gatelength/(double)CVMAX;

  key_events *v=&(m->input.keys.value);
  for(int i=0; i<v->len; ++i){
    int key=v->buf[i].key;
    if(v->buf[i].state==KEY_DOWN){
      m->held[key]=1;
      m->keys++;
    } else {
      m->held[key]=0;
      m->keys--;
    }
  }

  int pwl=(m->time)%(m->steps*ticksperbeat);
  int step=pwl/ticksperbeat;
  pwl%=ticksperbeat;

  int32_t k=0;
  if (m->num[step]>=0){
    m->recent=-1; // Reset old randomized key
    k=keyfind(m->num[step], m->keys, WRAP, m->held);
    if (k>=0)
      m->output.pitch.value=(k+m->ntransp[step]+12*m->otransp[step])*HALFNOTE;
  } else if (m->keys>0){
    int rk = rand()%m->keys;
    k=keyfind(rk, m->keys, WRAP, m->held);
    if (m->recent<0)
      m->recent=k;
    m->output.pitch.value=(m->recent+m->ntransp[step]+12*m->otransp[step])*HALFNOTE;
  }

  int gate=0;
  if(k>=0){
    if (pwl<(double)ticksperbeat*dt && m->keys>0 && m->on[step]){
      m->output.gate.value=1;
    } else {
      if (!m->hold[step])
	m->output.gate.value=0;
    }
  } else {
    m->output.gate.value=0;
  }

  set_output(&m->output.gate);
  set_output(&m->output.pitch);

  if (m->keys>0)
    m->time+=elapsed;
  else
    m->time=0;
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

  if (strcmp(argv[0], "hold")==0){
    for (int i = 0; i<m->steps; i++){
      long to;
      strtocv(argv[i+1], &to);
      m->hold[i] = to;
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
  printf("Hullo!\n");
  free(_m->on);
  free(_m->hold);
  free(_m->num);
  free(_m->ntransp);
  free(_m->otransp);
  printf("Hullo!\n");
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
  m->keys=0;
  m->recent=-1;
  for (int i=0; i<256; i++)
    m->held[i]=0;

  m->on=malloc(sizeof(int32_t)*m->steps);
  m->hold=malloc(sizeof(int32_t)*m->steps);
  m->num=malloc(sizeof(int32_t)*m->steps);
  m->ntransp=malloc(sizeof(int32_t)*m->steps);
  m->otransp=malloc(sizeof(int32_t)*m->steps);

  for (int i=0; i<m->steps; ++i){
    m->on[i]=0;
    m->hold[i]=0;
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
