#define _GNU_SOURCE

#include<time.h>
#include<pthread.h>
#include<sched.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/io.h>
#include<sys/mman.h>
#include<termios.h>


#define PERIOD 1000000L


// CL
// måste vara låg minst 5ms för att ladda, 6ms ger full amplitud
// går att trigga i stort sett hur fort som helst
// måste vara hög minst 13ms för att undvika dubbeltriggning

// D minst 5ms låg, 6ms max amplitud
// kortare laddpuls högre tonhöjd upp till ~50ms
// måste vara hög ~11ms för att undvika dubbeltriggning

// B minst 5ms låg, ~8ms max amplitud, kan triggas hur fort som helst
// dubbeltriggning märks inte

// BR minst 7ms låg, ger olika karaktär upp till ~20ms
// kan triggas fort, men låter olika beroende på intervallet
// olika lång hög puls ger olika karaktär; längre ger längre ljud
// 1ms -> 2ms redan stor skillnad, sedan alltmer knäppigt ljud

// L måste vara jordad eller flytande och påverkar ljudet på BR

// SD mycket svag vid 5ms laddning, tydlig vid 6ms, olika karaktär


#define SELECT_MODULE(n) outb(n, 0x1f1)
#define SELECT_REG(n) outb(n, 0x1f2)
#define WRITE(n) outw(n, 0x1f0)
#define READ() inw(0x1f0)

enum{CL_BIT=1, B_BIT=2, BR_BIT=4, D_BIT=8, SD_BIT=128, L_BIT=128};
volatile int cl_time, d_time, b_time, br_time, sd_time, l_state;

volatile int quit;

int cl_length=13, d_length=12, b_length=1, br_length=1, sd_length=1;

void *rt_thread(void *data){
  struct timespec next;

  SELECT_MODULE(0);

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1){
    int bits=!!cl_time*CL_BIT | !!d_time*D_BIT | !!b_time*B_BIT |
             !!br_time*BR_BIT | !!sd_time*SD_BIT;

    if(quit)return 0;

    if(cl_time)--cl_time;
    if(d_time)--d_time;
    if(b_time)--b_time;
    if(br_time)--br_time;
    if(sd_time)--sd_time;

    SELECT_REG(0);
    WRITE(!!l_state*L_BIT);
    SELECT_REG(1);
    WRITE(bits);

    next.tv_nsec+=PERIOD;
    while(next.tv_nsec>=1000000000){
      next.tv_sec++;
      next.tv_nsec-=1000000000;
      }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    }
  }

int main(){
  struct sched_param sched;
  pthread_attr_t attr;
  pthread_t thread;
  struct termios told, tnew;
  int c;

  tcgetattr(0, &told);
  tnew=told;
  tnew.c_lflag&=~(ICANON|ECHO|ISIG);
  tnew.c_cc[VMIN]=1; tnew.c_cc[VTIME]=0;
  tcsetattr(0, TCSANOW, &tnew);

  ioperm(0x1f0, 16, 1);

  if(mlockall(MCL_CURRENT|MCL_FUTURE)<0){
    fprintf(stderr, "mlockall(): %m\n");
    exit(1);
    }

  SELECT_MODULE(0);

  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 1024*1024);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  sched.sched_priority=80;
  pthread_attr_setschedparam(&attr, &sched);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_create(&thread, &attr, rt_thread, NULL);

  while((c=getchar())!='q'){
    switch(c){
      case 'a':
        cl_time=cl_length; break;
      case 's':
        d_time=d_length; break;
      case 'd':
        b_time=b_length; break;
      case 'f':
        br_time=br_length; break;
      case 'g':
        sd_time=sd_length; break;
      case 'l':
        l_state=1-l_state; printf("L is %s\n", l_state?"on":"off"); break;
      }
    }

  quit=1;
  pthread_join(thread, NULL);

  tcsetattr(0, TCSANOW, &told);

  return 0;
  }
