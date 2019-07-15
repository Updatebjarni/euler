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


struct debounce{
  int key, timer;
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
  for(int n=0; (n<16) && (!((*buf++=MODULE_READ())&0x8000)); ++n);
  }

volatile struct{int key; char ready;}single_grab;

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

  m->output.lower.value.len=mog_out_len;
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
  mog_out=m->output.lower.value.buf;
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
