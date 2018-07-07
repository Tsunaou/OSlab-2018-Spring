#ifndef __lib_h__
#define __lib_h__

typedef unsigned int   uint32_t;
typedef int sem_t;


void printf(const char *format,...);
int fork();
void sleep(uint32_t time);
int exit();

int sem_init(sem_t *sem, uint32_t value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_destroy(sem_t *sem);





#endif
