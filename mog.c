/*
*** kommentar om räknare för debouncing:

det behövs en räknare som innehåller tiokompisen till summan av tiderna
som är i kön för tillfället. den räknas upp med ett i slutet av varje
runda i debouncerrutinen, tills den når 10, då kön är tom.

när en ny räknare läggs till i kön sätts den till värdet från denna räknare,
som sätts till 0.

*/

#include<stdlib.h>
#include<unistd.h>
#include"orgel.h"
#include"orgel-io.h"

#include"mog.spec.c"

typedef struct mog_module{
  MODULE_BASE
  }mog_module;

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

static struct{
  unsigned short raw_bits[2];
  union{
    unsigned short cooked_bits[2];
    struct{
      union{
        unsigned short word;
        unsigned char byte[2];
        }data;
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

static void read_keys(){
  SELECT_MODULE(MOG_MODULE_NUMBER);
  MODULE_SET_REG(0);
  for(int i=0; i<10; ++i)
    keybits[current][i]=MODULE_READ();
  keybits[current][8]^=0xFFF;  // The pedal has the switches the other way,
  keybits[current][9]^=0xFFF;  // so the bits need to be inverted.
  for(int i=0; i<3; ++i){
    bschan[i].raw_bits[0]=MODULE_READ();
    bschan[i].raw_bits[1]=MODULE_READ();
    bschan[i].cooked_bits[0]=bschan[i].raw_bits[0]&0xFFF;
    bschan[i].cooked_bits[1]=bschan[i].raw_bits[1];
    bschan[i].cooked_bits[0]|=(bschan[i].data_high<<12);
    bschan[i].data_ready=!(bschan[i].raw_bits[0]&0x8000);

    }
  }

volatile struct{int key, keyboard; char ready;}single_grab;

static void tick(module *_m, int elapsed){
  mog_module *m=(mog_module *)_m;
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

  m->output.lower.value.len=buflen[LOWER];
  m->output.upper.value.len=buflen[UPPER];
  m->output.pedal.value.len=buflen[PEDAL];
// *** PROPAGATE OUTPUT

  current=!current;
  }

int mog_grab_key(){
  single_grab.ready=0;
  while(!single_grab.ready);
  return single_grab.key;
  }

class mog_class;

static module *create(char **argv){
  mog_module *m=malloc(sizeof(mog_module));
  base_module_init(m, &mog_class);
  eventbuf[LOWER]=m->output.lower.value.buf;
  eventbuf[UPPER]=m->output.upper.value.buf;
  eventbuf[PEDAL]=m->output.pedal.value.buf;
  read_keys(keybits[!current]);
  return (module *)m;
  }

class mog_class={
  .name="mog", .descr="Människa-Orgel-Gränssnittet",
  .tick=tick, .destroy=0, .config=0,
  .is_static=STATIC_CLASS,
  .create=create,
  .create_counter=0
  };
