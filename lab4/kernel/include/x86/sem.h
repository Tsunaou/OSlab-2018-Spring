#ifndef __SEM_H__
#define __SEM_H__

#include "common.h"

typedef int sem_t;

#define MAX_SEM_NUM 10

struct Semaphore {
    int value;
    int used;
    struct ProcessTable *list;
};

struct Semaphore Sem[MAX_SEM_NUM];

void reset_sem(int i);
void init_sem();
int new_sem();

#endif