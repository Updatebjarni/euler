#include<math.h>
#include<stdlib.h>
#include"orgel.h"
#include"orgel-io.h"

#include"sid.spec.c"


typedef struct sid_module{
  MODULE_BASE
  }sid_module;

static int release_table[16]={
  6, 24, 48, 72, 114, 168, 204, 240,
  300, 750, 1500, 2400, 3000, 9000, 15000, 24000
  };

static struct{
  struct{
    union{struct{unsigned decay:4, attack:4;}; unsigned ad;};
    union{struct{unsigned release:4, sustain:4;}; unsigned sr;};
    union{struct{unsigned gate:1, sync:1, ring:1, test:1,
                    triangle:1, saw:1, pulse:1, noise:1;};
          unsigned ctrl;};
    union{struct{unsigned pw_lo:8, pw_hi:8;}; unsigned pw;};
    union{struct{unsigned freq_lo:8, freq_hi:8;}; unsigned freq;};
    int pw_changed, freq_changed;
int release_timeout; 
   }voice[3];
  union{struct{unsigned vol:4, lp:1, bp:1, hp:1, mute3:1;}; unsigned modevol;};
  union{struct{unsigned filter:3, filtext:1, res:4;}; unsigned resfilt;};
  union{struct{unsigned fc_lo:3, fc_hi:8;}; unsigned cutoff;};
  }chip[3];

enum{REG_FREQ_LO, REG_FREQ_HI, REG_PW_LO, REG_PW_HI, REG_CTRL, REG_AD, REG_SR,
     REG_CUTOFF_LO=0x15, REG_CUTOFF_HI, REG_RESFILT, REG_MODEVOL};

static void write_chip(int chip, int reg, unsigned char val){
  MODULE_WRITE((chip<<13)|(reg<<8)|val);
  MODULE_READ();
  }

static void write_voice(int chip, int voice, int reg, unsigned char val){
  write_chip(chip, voice*7+reg, val);
  }

static void tick(module *_m, int elapsed){
  sid_module *m=(sid_module *)_m;
  SELECT_MODULE(1);
  for(int chipno=0; chipno<3; ++chipno){
    struct input_chip *chipjacks=&(m->input.chip[chipno]);
    chip[chipno].vol=chipjacks->vol.value;
    chip[chipno].hp=chipjacks->hp.value;
    chip[chipno].bp=chipjacks->bp.value;
    chip[chipno].lp=chipjacks->lp.value;
    chip[chipno].mute3=chipjacks->mute3.value;
    chip[chipno].filtext=chipjacks->filtext.value;
    chip[chipno].res=chipjacks->res.value;
    chip[chipno].cutoff=chipjacks->cutoff.value;
      
    for(int voiceno=0; voiceno<3; ++voiceno){
      struct input_chip_voice *voice=
        &(m->input.chip[chipno].voice[voiceno]);
      chip[chipno].voice[voiceno].pulse=voice->pulse.value;
      chip[chipno].voice[voiceno].triangle=voice->triangle.value;
      chip[chipno].voice[voiceno].saw=voice->saw.value;
      chip[chipno].voice[voiceno].noise=voice->noise.value;
      chip[chipno].voice[voiceno].pw=voice->pw.value;
      if(voice->pitch.valchanged){
        int pitch=voice->pitch.value;
        pitch=pow(2, (1.0*pitch)/OCTAVE)*115.25;
        chip[chipno].voice[voiceno].freq=pitch;
        voice->pitch.valchanged=0;
        }
      chip[chipno].voice[voiceno].attack=voice->attack.value;
      chip[chipno].voice[voiceno].decay=voice->decay.value;
      chip[chipno].voice[voiceno].sustain=voice->sustain.value;
      chip[chipno].voice[voiceno].release=voice->release.value;

      chip[chipno].voice[voiceno].ring=voice->ringmod.value;
      chip[chipno].voice[voiceno].sync=voice->sync.value;
      if(voice->filter.valchanged){
        chip[chipno].filter&=~(1<<voiceno);
        chip[chipno].filter|=((voice->filter.value)<<voiceno);
        voice->filter.valchanged=0;
        }
      chip[chipno].voice[voiceno].gate=voice->gate.value;
      if(chip[chipno].voice[voiceno].gate)
        chip[chipno].voice[voiceno].release_timeout=
          release_table[chip[chipno].voice[voiceno].release];
      else{
        if(chip[chipno].voice[voiceno].release_timeout)
          --chip[chipno].voice[voiceno].release_timeout;
        else
          chip[chipno].voice[voiceno].ctrl&=0x0F;
        }

      write_voice(chipno, voiceno, REG_FREQ_LO,
                  chip[chipno].voice[voiceno].freq_lo);
      write_voice(chipno, voiceno, REG_FREQ_HI,
                  chip[chipno].voice[voiceno].freq_hi);
      write_voice(chipno, voiceno, REG_PW_LO,
                  chip[chipno].voice[voiceno].pw_lo);
      write_voice(chipno, voiceno, REG_PW_HI,
                  chip[chipno].voice[voiceno].pw_hi);
      write_voice(chipno, voiceno, REG_CTRL,
                  chip[chipno].voice[voiceno].ctrl);
      write_voice(chipno, voiceno, REG_AD,
                  chip[chipno].voice[voiceno].ad);
      write_voice(chipno, voiceno, REG_SR,
                  chip[chipno].voice[voiceno].sr);
      }
    write_chip(chipno, REG_MODEVOL, chip[chipno].modevol);
    write_chip(chipno, REG_RESFILT, chip[chipno].resfilt);
    write_chip(chipno, REG_CUTOFF_LO, chip[chipno].fc_lo);
    write_chip(chipno, REG_CUTOFF_HI, chip[chipno].fc_hi);
    }
  }

static void debug(module *_m){
  for(int chipno=0; chipno<3; ++chipno){
    printf("Chip %d:  MODEVOL=%.2X  RESFILT=%.2X CUTOFF=%.2X:%.2X\n",
           chipno, chip[chipno].modevol, chip[chipno].resfilt,
           chip[chipno].fc_hi, chip[chipno].fc_lo);
    for(int voiceno=0; voiceno<3; ++voiceno){
      printf("  Voice %d:  AD=%.2X  SR=%.2X  CTRL=%.2X"
             "  PW=%.2X:%.2X  FREQ=%.2X:%.2X\n",
             voiceno,
             chip[chipno].voice[voiceno].ad,
             chip[chipno].voice[voiceno].sr,
             chip[chipno].voice[voiceno].ctrl,
             chip[chipno].voice[voiceno].pw_hi,
             chip[chipno].voice[voiceno].pw_lo,
             chip[chipno].voice[voiceno].freq_hi,
             chip[chipno].voice[voiceno].freq_lo);
      }
    }
  }

static void reset(module *m){
  LOCK_NEST();
  SELECT_MODULE(1);
  write_chip(7, 0, 0x00);
  for(int chip=0; chip<3; ++chip){
    write_chip(chip, REG_MODEVOL, 0x00);
    write_chip(chip, REG_RESFILT, 0x00);
    for(int voice=0; voice<3; ++voice){
      write_voice(chip, voice, REG_AD, 0x00);
      write_voice(chip, voice, REG_SR, 0x00);
      write_voice(chip, voice, REG_CTRL, 0x00);
      }
    }
  LOCK_UNNEST();
  }

class sid_class;

static module *create(char **argv){
  sid_module *m=malloc(sizeof(sid_module));
  base_module_init(m, &sid_class);
  return (module *)m;
  }

class sid_class={
  .name="sid", .descr="SID",
  .tick=tick, .destroy= 0, .config=0,
  .is_static=STATIC_CLASS,
  .create=create,
  .create_counter=0,
  .debug=debug,
  .reset=reset
  };
