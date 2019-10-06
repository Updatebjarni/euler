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
#include<fcntl.h>
#include<poll.h>
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

static unsigned between;

static int dialbox_fd=-1;
static struct pollfd dialbox_pfd={.events=POLLIN};
static int dialbox_buflen;
static unsigned char dialbox_buf[3];
static unsigned short dialbox_dial[8];
static long dialbox_val[8];

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

  if(dialbox_fd>-1){
    int res=poll(&dialbox_pfd, 1, 0);
    if(res<0 || dialbox_pfd.revents&POLLERR){
      close(dialbox_fd);
      dialbox_fd=-1;
      }
    if(dialbox_pfd.revents&POLLIN){
      res=read(dialbox_fd, dialbox_buf+dialbox_buflen, 3-dialbox_buflen);
      if(res<0){
        close(dialbox_fd);
        dialbox_fd=-1;
        }
      else{
        dialbox_buflen+=res;
        if(dialbox_buflen==3){
          dialbox_buflen=0;
          short dialno=dialbox_buf[0]&7;
          unsigned short pos=(dialbox_buf[1]<<8)|dialbox_buf[2];
          short delta=pos-dialbox_dial[dialno];
          dialbox_dial[dialno]=pos;
          dialbox_val[dialno]+=delta;
          m->output.dial[dialno].value=
            dialbox_val[dialno]*m->input.dial_res[dialno].value;
          set_output(&m->output.dial[dialno]);
printf("dialbox: val %ld, out %ld\n", dialbox_val[dialno], (long)m->output.dial[dialno].value);
//          printf("between: %d\n", between);
//          between=0;
          }
        }
      }
    }
//if(between<0xFFFFFFFF)++between;

  m->output.lower.value.len=buflen[LOWER];
  m->output.upper.value.len=buflen[UPPER];
  m->output.pedal.value.len=buflen[PEDAL];
  set_output(&m->output.pedal);
  set_output(&m->output.upper);
  set_output(&m->output.lower);

  current=!current;
  }

int mog_grab_key(){
  single_grab.ready=0;
  while(!single_grab.ready);
  return single_grab.key;
  }

void plugstatus(module *_m, jack *j){
  mog_module *m=(mog_module *)_m;

  if(j->parent_jack==(jack *)m->output.dial){
    int n=j-(jack *)(m->output.dial);
    if(j->nconnections)
      dialbox_val[n]=
        200LL*j->connections[0]->value.virtual_cv/m->input.dial_res[n].value;
    fprintf(stderr, "dialbox output %d (un)plugged\n", n);
    }
  }

class mog_class;

static module *create(char **argv){
  mog_module *m=malloc(sizeof(mog_module));
  base_module_init(m, &mog_class);
  eventbuf[LOWER]=m->output.lower.value.buf;
  eventbuf[UPPER]=m->output.upper.value.buf;
  eventbuf[PEDAL]=m->output.pedal.value.buf;
  read_keys(keybits[!current]);

  int dfd=open("/dev/ttyS0", O_RDWR|O_NOCTTY);
  if(dfd<0)perror("(mog dialbox) /dev/ttyS0");
  else{
    static unsigned char cmd_init[]={0x20},
                         cmd_enable[]={0x50, 0x00, 0xFF},
                         response;
    write(dfd, cmd_init, 1);
    dialbox_pfd.fd=dfd;
    int res=poll(&dialbox_pfd, 1, 500);
    if(!res)fprintf(stderr, "(mog dialbox) poll timeout\n");
    else if(res<0 || dialbox_pfd.revents&POLLERR)perror("(mog dialbox) poll");
    else if(read(dfd, &response, 1)<0)perror("(mog dialbox) read");
    else if(response!=0x20)fprintf(stderr, "(mog dialbox) bad response\n");
    else if(write(dfd, cmd_enable, 3)<0)perror("(mog dialbox) write");
    else dialbox_fd=dfd;
    }
  for(int i=0; i<8; ++i)
    m->input.dial_res[i].value=1*VOLT;

  return (module *)m;
  }

class mog_class={
  .name="mog", .descr="Människa-Orgel-Gränssnittet",
  .tick=tick, .destroy=0, .config=0,
  .plugstatus=plugstatus,
  .is_static=STATIC_CLASS,
  .create=create,
  .create_counter=0
  };
