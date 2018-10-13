#include<stdlib.h>
#include<unistd.h>
#include"orgel.h"
#include"orgel-io.h"

#include"mog.spec.c"

#define DEBOUNCE_TIME 10
#define MOG_MODULE_NUMBER 16


enum{LOWER=0, UPPER, PEDAL};

struct debounce{
  int key, timer;
  };

static unsigned short keybits[2][10];
static unsigned short bouncemask[10]={0};
static struct debounce timers[12*10]={{-1, 0}};
static int timers_head=0, timers_tail=0;
#define HEAD timers[timers_head]

static struct key_event *eventbuf[3];
static int current=1;

static union{
  unsigned short raw_bits[2];
  struct{
    union{
      unsigned short word;
      unsigned char byte[2];
      }data;
    struct{
      unsigned short
        from       :6,
        last_part  :1,
        is_reply   :1,
        data_high  :4,
        _          :3,
        data_ready :1;
      };
    };
  }bschan[3];

static void read_keys(unsigned short *buf){
  SELECT_MODULE(MOG_MODULE_NUMBER);
  MODULE_SET_REG(0);
  for(int i=0; i<10; ++i)
    keybits[current][i]=MODULE_READ();
  keybits[current][8]^=0xFFF;  // The pedal has the switches the other way,
  keybits[current][9]^=0xFFF;  // so the bits need to be inverted.
  for(int i=0; i<3; ++i){
    bschan[i].raw_bits[0]=MODULE_READ();
    bschan[i].raw_bits[1]=MODULE_READ();
    bschan[i].data.word&=0xFFF;
    bschan[i].data.word|=(bschan[i].data_high<<12);
    bschan[i].data_ready^=1;  // It's an active low signal, you see.
    }
  }

volatile struct{int key, keyboard; char ready;}single_grab;

static void tick(module *m, int elapsed){
  if(HEAD.key!=-1){
    if(!--HEAD.timer){
      bouncemask[HEAD.key/12]&=~(1<<(HEAD.key%12));
      HEAD.key=-1;
      timers_head=(timers_head+1)%(12*10);
      }
    }

  read_keys(keybits[current]);

  int buflen[3]={0, 0, 0};

  for(int i=0; i<10; ++i){
    unsigned short old, new, changed;

    old=keybits[!current][i];
    new=keybits[current][i];
    new&=~bouncemask[i]; new|=(old&bouncemask[i]);
    keybits[current][i]=new;
    changed=(new^old)&0xFFF;

    if(changed){
      for(int key=0; changed; ++key, changed>>=1){
        int keynum=i*12+key, keyboard;
        if(keynum<48){keyboard=LOWER; keynum+=44;}
        else if(keynum<96){keyboard=UPPER; keynum+=8;}
        else{keyboard=PEDAL; keynum-=76;}
        if(changed&1){
          if(!(new&(1<<key))){
            single_grab.key=keynum;
            single_grab.keyboard=keyboard;
            single_grab.ready=1;
            }
          eventbuf[keyboard][buflen[keyboard]].key=keynum;
          eventbuf[keyboard][buflen[keyboard]].state=!!(new&(1<<key));
          ++buflen[keyboard];

          bouncemask[i]|=(1<<key);
          timers[timers_tail].key=i*12+key;
          timers[timers_tail].timer=DEBOUNCE_TIME;  // *** inte riktigt
          timers_tail=(timers_tail+1)%(12*10);
          timers[timers_tail].key=-1;
          }
        }
      }
    }

  OUTPUT(m)->lower.key_events_value.len=buflen[LOWER];
  OUTPUT(m)->upper.key_events_value.len=buflen[UPPER];
  OUTPUT(m)->pedal.key_events_value.len=buflen[PEDAL];
  current=!current;
  }

int mog_grab_key(){
  single_grab.ready=0;
  while(!single_grab.ready);
  return single_grab.key;
  }

class mog_class;

static module *create(char **argv){
  module *m=malloc(sizeof(module));
  default_module_init(m, &mog_class);
  eventbuf[0]=OUTPUT(m)->lower.key_events_value.buf;
  eventbuf[1]=OUTPUT(m)->upper.key_events_value.buf;
  eventbuf[2]=OUTPUT(m)->pedal.key_events_value.buf; 
  OUTPUT(m)->lower.changed=1;
  OUTPUT(m)->upper.changed=1;
  OUTPUT(m)->pedal.changed=1;
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
