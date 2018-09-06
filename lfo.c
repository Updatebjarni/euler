#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"lfo.spec.c"

typedef struct lfo_module{
  module;
  int time;
  }lfo_module;

static int32_t square(int time, int amp, int duty, int per){
  return (time%per<duty) ? amp:-amp;
  }

static int32_t sine(int time, int amp, int per){
  return amp*sin((2.0*M_PI*time)/per);
  }

static int32_t triangle(int time, int amp, int per){
  return 0;
  }

static int32_t sawtooth(int time, int amp, int per){
  return 0;
  }

static void tick(module *_m, int elapsed){
  lfo_module *m=(lfo_module *)_m;
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;
  if(in->frequency.connection && in->amplitude.connection && in->waveform.connection){
    int32_t frequency=(in->frequency.connection->out_terminal.value.int32);
    int32_t amplitude=(in->amplitude.connection->out_terminal.value.int32);
    int32_t waveform=(in->waveform.connection->out_terminal.value.int32);

    // Rewrite the following
    int32_t length=1000000/frequency;
    int32_t value=0;

    switch(waveform) {
      case 1:
        value=triangle(m->time, amplitude, length);
        break;
      case 2:
        value=sawtooth(m->time, amplitude, length);
        break;
      case 3:
        value=square(m->time, amplitude, length/2, length);
        break;
      default:
        value=sine(m->time, amplitude, length);
      }
    
    m->time+=elapsed;
    m->output.out_terminal.value.int32=value;
    }
  }

class lfo_class;

static module *create(char **argv){
  lfo_module *m;

  m=malloc(sizeof(lfo_module));
  default_module_init((module *)m, &lfo_class);
  m->time=0;
  return (module *)m;
  }

class lfo_class={
  "lfo",                     // char *name
  "Low frequency oscillator",// char *descr
  &input,                    // jack *default_input
  &output,                   // jack *default_output
  tick,                      // void (*default_tick)(module *, int elapsed)
  0,                         // void (*default_destroy)(module *)
  0,                         // void (*default_config)(module *, char **)
  DYNAMIC_CLASS,             // int is_static
  create,                    // module*(*create)(char **)
  0                          // int create_counter
  };