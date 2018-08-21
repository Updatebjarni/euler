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


#define SELECT_MODULE(n) outb(n, 0x1f1)
#define SELECT_REG(n) outb(n, 0x1f2)
#define WRITE(n) outw(n, 0x1f0)
#define READ() inw(0x1f0)

int main(){
  ioperm(0x1f0, 16, 1);

  if(mlockall(MCL_CURRENT|MCL_FUTURE)<0){
    fprintf(stderr, "mlockall(): %m\n");
    exit(1);
    }

  while(1)SELECT_MODULE(0);

  return 0;
  }
