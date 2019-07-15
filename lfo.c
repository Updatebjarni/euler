#define _GNU_SOURCE

#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include"orgel.h"

#include"lfo.spec.c"

typedef struct lfo_module{
  MODULE_BASE
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
  return -amp + (time % per) * (2.0*amp)/per;
  }

static void tick(module *_m, int elapsed){
  lfo_module *m=(lfo_module *)_m;

  int32_t frequency=m->input.frequency.value;
  int32_t amplitude=m->input.amplitude.value;
  int32_t waveform=m->input.waveform.value;

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
  m->output.value=value;
  }

class lfo_class;

static module *create(char **argv){
  lfo_module *m=malloc(sizeof(lfo_module));
  base_module_init(m, &lfo_class);
  m->time=0;
  return (module *)m;
  }

class lfo_class={
  .name="lfo",
  .descr="Low frequency oscillator",
  .tick=tick,
  .is_static=DYNAMIC_CLASS,
  .create=create,
  };
