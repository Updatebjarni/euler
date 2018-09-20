#include<stdlib.h>
#include<unistd.h>
#include"orgel.h"
#include"orgel-io.h"

#include"mog.spec.c"

#define DEBOUNCE_TIME 10
#define MOG_MODULE_NUMBER 16


struct debounce{
  char key, timer;
  };

static unsigned short keybits[2][16];
static unsigned short bouncemask[16]={0};
static struct debounce timers[12*10]={{-1, 0}};
static int timers_head=0, timers_tail=0;
#define HEAD timers[timers_head]

static struct key_event *mog_out;
static int current=1;

static void read_keys(unsigned short *buf){
  SELECT_MODULE(MOG_MODULE_NUMBER);
  MODULE_SET_REG(0);
  while(!((*buf++=MODULE_READ())&0x8000));
  }

volatile struct{char key; char ready;}single_grab;

static void tick(module *m, int elapsed){
  if(HEAD.key!=-1){
    if(!--HEAD.timer){
      bouncemask[HEAD.key/12]&=~(1<<(HEAD.key%12));
      HEAD.key=-1;
      timers_head=(timers_head+1)%(12*10);
      }
    }

  read_keys(keybits[current]);

  int mog_out_len=0;

  for(int i=0; i<10; ++i){
    unsigned short old, new, changed;

    old=keybits[!current][i];
    new=keybits[current][i];
    new&=~bouncemask[i]; new|=(old&bouncemask[i]);
    keybits[current][i]=new;
    changed=(new^old)&0xFFF;

    if(changed){
      for(int key=0; changed; ++key, changed>>=1){
        int keynum=i*12+key+44;
        if(changed&1){
          if(!(new&(1<<key))){
            single_grab.key=keynum;
            single_grab.ready=1;
            }
          mog_out[mog_out_len].key=keynum;
          mog_out[mog_out_len].state=!!(new&(1<<key));
          ++mog_out_len;

          bouncemask[i]|=(1<<key);
          timers[timers_tail].key=i*12+key;
          timers[timers_tail].timer=DEBOUNCE_TIME;  // *** inte riktigt
          timers_tail=(timers_tail+1)%(12*10);
          timers[timers_tail].key=-1;
          }
        }
      }
    }

  OUTPUT(m)->lower.key_events_value.len=mog_out_len;
  current=!current;
  }

int mog_grab_key(){
  single_grab.ready=0;
  while(!single_grab.ready);
  return single_grab.key;
  }

// TODO: splitting! redo mog so one can create one module per key group

/*
struct key_event left_out[12*10];
int left_out_len;
struct key_event right_out[12*10];
int right_out_len;

static int split=-1;
static int transpose_left, transpose_right;

void split_keys(){
  left_out_len=0; right_out_len=0;
  for(int i=0; i<mog_out_len; ++i){
    if(mog_out[i].key>split){
      right_out[right_out_len]=mog_out[i];
      right_out[right_out_len].key+=transpose_right;
      ++right_out_len;
      }
    else{
      left_out[left_out_len]=mog_out[i];
      left_out[left_out_len].key+=transpose_left;
      ++left_out_len;
      }
    }
  }
*/

class mog_class;

static module *create(char **argv){
  module *m=malloc(sizeof(module));
  default_module_init(m, &mog_class);
  mog_out=OUTPUT(m)->lower.key_events_value.buf;
  OUTPUT(m)->lower.changed=1;
  read_keys(keybits[!current]);
  return m;
  }

class mog_class={
  "mog", "Människa-Orgel-Gränssnittet",
  &input, &output,
  tick, 0, 0,
  STATIC_CLASS,
  create,
  0
  };
