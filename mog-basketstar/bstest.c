#include<ctype.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include"../orgel.h"
#include"../orgel-io.h"

#define MOG_MODULE_NUMBER 16


static unsigned short keybits[2][10];

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
  for(int i=0; i<10; ++i){
    keybits[current][i]=MODULE_READ();
//    printf("%04X ", keybits[current][i]);
    }
//  putchar('\n');
  keybits[current][8]^=0xFFF;  // The pedal has the switches the other way,
  keybits[current][9]^=0xFFF;  // so the bits need to be inverted.
  for(int i=0; i<3; ++i){
    bschan[i].raw_bits[0]=MODULE_READ();
    bschan[i].raw_bits[1]=MODULE_READ();
    bschan[i].cooked_bits[0]=bschan[i].raw_bits[0]&0xFFF;
    bschan[i].cooked_bits[1]=bschan[i].raw_bits[1];
    bschan[i].cooked_bits[0]|=(bschan[i].data_high<<12);
    bschan[i].data_ready=!(bschan[i].raw_bits[0]&0x8000);

//printf("%04X %04X ", bschan[i].raw_bits[0], bschan[i].raw_bits[1]);
//    bschan[i].data.word&=0xFFF;
//    bschan[i].data.word|=(bschan[i].data_high<<12);
//    bschan[i].data_ready^=1;  // It's an active low signal, you see.
    }
//putchar('\n');
  }

char **orgelperm_argv;

void require_orgelperm(int foo){
  execv(orgelperm_argv[0], orgelperm_argv);
  }

void out(unsigned char c){
  if(isprint(c))printf("'%c'", c);
  else printf("???");
  printf(" (%2X) ", c);
  }

int main(int argc, char *argv[]){
  orgelperm_argv=malloc(sizeof(argv[0])*(argc+2));
  memcpy(orgelperm_argv+1, argv, sizeof(argv[0])*(argc+1));
  orgelperm_argv[0]="/usr/bin/orgelperm";
  signal(SIGSEGV, require_orgelperm);
  SELECT_MODULE(0);

  while(1){
    read_keys();
    if(bschan[0].data_ready){
      printf("%02X %02X %02X %04X\n", bschan[0].raw_bits[1]&255,
                                 bschan[0].data.byte[0],
                                 bschan[0].data.byte[1],
                                 bschan[0].data.word);
//      out(bschan[0].data.byte[0]);
//      out(bschan[0].data.byte[1]);
//      out(bschan[0].raw_bits[1]&255);
//      putchar('\n');
      }
    for(int i=0; i<10000000; ++i);
    }
  }
