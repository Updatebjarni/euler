#ifndef ORGEL_IO_H
#define ORGEL_IO_H

#ifdef NO_HARDWARE
#define HW(x) 0
#else
#include<sys/io.h>
#define HW(x) x
#endif

#define ORGEL_IO_PORTS_BASE  0x1f0
#define ORGEL_IO_PORTS_LEN   3

#define ORGEL_MODPORT  (ORGEL_IO_PORTS_BASE + 1)
#define ORGEL_REGPORT  (ORGEL_IO_PORTS_BASE + 2)
#define ORGEL_DATAPORT (ORGEL_IO_PORTS_BASE + 0)

#define SELECT_MODULE(n)  HW(outb(n, ORGEL_MODPORT))
#define MODULE_SET_REG(n) HW(outb(n, ORGEL_REGPORT))
#define MODULE_WRITE(n)   HW(outw(n, ORGEL_DATAPORT))
#define MODULE_READ()     HW(inw(ORGEL_DATAPORT))

#endif
