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
  struct input_bundle *in=(struct input_bundle *)m->input.bundle.elements;
  for(int chipno=0; chipno<3; ++chipno){
    if(in->chip[chipno].bundle->vol.connection){
      chip[chipno].vol=
        in->chip[chipno].bundle->vol.connection->out_terminal.value.int32;
      }
    for(int voiceno=0; voiceno<3; ++voiceno){
      if(in->chip[chipno].bundle->voice[voiceno].bundle->pulse.connection)
        chip[chipno].voice[voiceno].pulse=in->chip[chipno].bundle->voice[voiceno].bundle->pulse.connection->out_terminal.value.bool;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->triangle.connection)
        chip[chipno].voice[voiceno].triangle=in->chip[chipno].bundle->voice[voiceno].bundle->triangle.connection->out_terminal.value.bool;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->saw.connection)
        chip[chipno].voice[voiceno].saw=in->chip[chipno].bundle->voice[voiceno].bundle->saw.connection->out_terminal.value.bool;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->noise.connection)
        chip[chipno].voice[voiceno].noise=in->chip[chipno].bundle->voice[voiceno].bundle->noise.connection->out_terminal.value.bool;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->pw.connection)
        chip[chipno].voice[voiceno].pw=in->chip[chipno].bundle->voice[voiceno].bundle->pw.connection->out_terminal.value.int32;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->pitch.connection){
        int pitch=in->chip[chipno].bundle->voice[voiceno].bundle->pitch.connection->out_terminal.value.int32;
        pitch=pow(2, (1.0*pitch)/OCTAVE)*115.25;
        chip[chipno].voice[voiceno].freq=pitch;
        }
      if(in->chip[chipno].bundle->voice[voiceno].bundle->attack.connection)
        chip[chipno].voice[voiceno].attack=in->chip[chipno].bundle->voice[voiceno].bundle->attack.connection->out_terminal.value.int32;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->decay.connection)
        chip[chipno].voice[voiceno].decay=in->chip[chipno].bundle->voice[voiceno].bundle->decay.connection->out_terminal.value.int32;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->sustain.connection)
        chip[chipno].voice[voiceno].sustain=in->chip[chipno].bundle->voice[voiceno].bundle->sustain.connection->out_terminal.value.int32;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->release.connection)
        chip[chipno].voice[voiceno].release=in->chip[chipno].bundle->voice[voiceno].bundle->release.connection->out_terminal.value.int32;

      if(in->chip[chipno].bundle->voice[voiceno].bundle->ringmod.connection)
        chip[chipno].voice[voiceno].ring=in->chip[chipno].bundle->voice[voiceno].bundle->ringmod.connection->out_terminal.value.bool;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->sync.connection)
        chip[chipno].voice[voiceno].sync=in->chip[chipno].bundle->voice[voiceno].bundle->sync.connection->out_terminal.value.bool;
      if(in->chip[chipno].bundle->voice[voiceno].bundle->filter.connection){
        chip[chipno].filter&=~(1<<voiceno);
        chip[chipno].filter|=((in->chip[chipno].bundle->voice[voiceno].bundle->filter.connection->out_terminal.value.bool)<<voiceno);
        }
      if(in->chip[chipno].bundle->voice[voiceno].bundle->gate.connection)
        chip[chipno].voice[voiceno].gate=in->chip[chipno].bundle->voice[voiceno].bundle->gate.connection->out_terminal.value.bool;
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

static module *create(char **argv){
  module *m=malloc(sizeof(module));
  default_module_init(m, &sid_class);
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
  return m;
  }

class sid_class={
  "sid", "SID",
  &input, &output,
  tick, 0, 0,
  STATIC_CLASS,
  create,
  0
  };
