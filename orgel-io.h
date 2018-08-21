#ifndef ORGEL_IO_H
#define ORGEL_IO_H

#include<sys/io.h>

#define ORGEL_IO_PORTS_BASE  0x1f0
#define ORGEL_IO_PORTS_LEN   3

#define ORGEL_MODPORT  (ORGEL_IO_PORTS_BASE + 1)
#define ORGEL_REGPORT  (ORGEL_IO_PORTS_BASE + 2)
#define ORGEL_DATAPORT (ORGEL_IO_PORTS_BASE + 0)

#define SELECT_MODULE(n)  outb(n, ORGEL_MODPORT)
#define MODULE_SET_REG(n) outb(n, ORGEL_REGPORT)
#define MODULE_WRITE(n)   outw(n, ORGEL_DATAPORT)
#define MODULE_READ()     inw(ORGEL_DATAPORT)

#endif
