#ifndef __CONST_H__
#define __CONST_H__

#define TRUE         1
#define FALSE        0

#define NULL         ((void*)0)

#define SYS_write   4     // defined in <sys/syscall.h>
#define SYS_fork    2
#define SYS_exit    1
#define SYS_nanosleep   162
#define SYS_sem_init    5
#define SYS_sem_post    6
#define SYS_sem_wait    7
#define SYS_sem_destroy 8

#endif
