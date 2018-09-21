#include<math.h>
#include<stdlib.h>
#include"orgel.h"
#include"orgel-io.h"

#include"sid.spec.c"

static struct{
  struct{
    union{struct{unsigned decay:4, attack:4;}; unsigned ad;};
    union{struct{unsigned release:4, sustain:4;}; unsigned sr;};
    union{struct{unsigned gate:1, sync:1, ring:1, test:1,
                    triangle:1, saw:1, pulse:1, noise:1;};
          unsigned ctrl;};
    int ad_changed, sr_changed, ctrl_changed;
    union{struct{unsigned pw_lo:8, pw_hi:8;}; unsigned pw;};
    union{struct{unsigned freq_lo:8, freq_hi:8;}; unsigned freq;};
    int pw_changed, freq_changed;
    }voice[3];
  union{struct{unsigned vol:4, lp:1, bp:1, hp:1, mute3:1;}; unsigned modevol;};
  union{struct{unsigned filter:3, filtext:1, res:4;}; unsigned resfilt;};
  int modevol_changed, resfilt_changed;
  union{struct{unsigned fc_lo:3, fc_hi:8;}; unsigned cutoff;};
  int cutoff_changed;
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

static void tick(module *m, int elapsed){
  SELECT_MODULE(1);
  for(int chipno=0; chipno<3; ++chipno){
    struct input_chip_bundle *chipjacks=INPUT(m)->chip.INDEX(chipno);
    if(chipjacks->vol.connection)
      chip[chipno].vol=chipjacks->vol.connection->value;
    if(chipjacks->hp.connection)
      chip[chipno].hp=chipjacks->hp.connection->value;
    if(chipjacks->bp.connection)
      chip[chipno].bp=chipjacks->bp.connection->value;
    if(chipjacks->lp.connection)
      chip[chipno].lp=chipjacks->lp.connection->value;
    if(chipjacks->mute3.connection)
      chip[chipno].mute3=chipjacks->mute3.connection->value;
    if(chipjacks->filtext.connection)
      chip[chipno].filtext=chipjacks->filtext.connection->value;
    if(chipjacks->res.connection)
      chip[chipno].res=chipjacks->res.connection->value;
    if(chipjacks->cutoff.connection)
      chip[chipno].cutoff=chipjacks->cutoff.connection->value;
      
    for(int voiceno=0; voiceno<3; ++voiceno){
      struct input_chip_voice_bundle *voice=
        INPUT(m)->chip.INDEX(chipno)->voice.INDEX(voiceno);
      if(voice->pulse.connection)
        chip[chipno].voice[voiceno].pulse=voice->pulse.connection->value;
      if(voice->triangle.connection)
        chip[chipno].voice[voiceno].triangle=voice->triangle.connection->value;
      if(voice->saw.connection)
        chip[chipno].voice[voiceno].saw=voice->saw.connection->value;
      if(voice->noise.connection)
        chip[chipno].voice[voiceno].noise=voice->noise.connection->value;
      if(voice->pw.connection)
        chip[chipno].voice[voiceno].pw=voice->pw.connection->value;
      if(voice->pitch.connection){
        int pitch=voice->pitch.connection->value;
        pitch=pow(2, (1.0*pitch)/OCTAVE)*115.25;
        chip[chipno].voice[voiceno].freq=pitch;
        }
      if(voice->attack.connection)
        chip[chipno].voice[voiceno].attack=voice->attack.connection->value;
      if(voice->decay.connection)
        chip[chipno].voice[voiceno].decay=voice->decay.connection->value;
      if(voice->sustain.connection)
        chip[chipno].voice[voiceno].sustain=voice->sustain.connection->value;
      if(voice->release.connection)
        chip[chipno].voice[voiceno].release=voice->release.connection->value;

      if(voice->ringmod.connection)
        chip[chipno].voice[voiceno].ring=voice->ringmod.connection->value;
      if(voice->sync.connection)
        chip[chipno].voice[voiceno].sync=voice->sync.connection->value;
      if(voice->filter.connection){
        chip[chipno].filter&=~(1<<voiceno);
        chip[chipno].filter|=((voice->filter.connection->value)<<voiceno);
        }
      if(voice->gate.connection)
        chip[chipno].voice[voiceno].gate=voice->gate.connection->value;
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
    }
  }

class sid_class;

static void debug(module *_m){
  for(int chipno=0; chipno<3; ++chipno){
    printf("Chip %d:  MODEVOL=%.2X  RESFILT=%.2X\n",
           chipno, chip[chipno].modevol, chip[chipno].resfilt);
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

static void init(module *m){
  LOCK_HARDWARE();
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
  UNLOCK_HARDWARE();
  }

static module *create(char **argv){
  module *m=malloc(sizeof(module));
  default_module_init(m, &sid_class);
  init(m);
  return m;
  }

class sid_class={
  "sid", "SID",
  &input, &output,
  tick, 0, 0,
  STATIC_CLASS,
  create,
  0,
  debug,
  0,
  init
  };
