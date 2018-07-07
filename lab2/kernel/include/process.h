#ifndef __PCB_H__
#define __PCB_H__

#include "list.h"

#define MAX_PCB_SIZE 10
#define MAX_STACK_SIZE 4096
#define TIMECOUNT 10

typedef struct PCB {
	struct TrapFrame *regs;
	uint32_t kstack[MAX_STACK_SIZE];
	list_head state_list;
	enum {
		RUNNABLE, RUNNING, BLOCKED, FREE
	} state;
	int timeCount;
	int sleepTime;
	uint32_t pid;
} PCB;

typedef struct record {
	uint32_t index, base;
} record;
extern record pc_rec;

extern list_head runnable, blocked, free;
extern PCB *current;
extern list_head *next;
extern PCB pcb_pool[];
extern uint8_t schedule_flag;

void initProcess(uint32_t);
PCB *new_pcb();
void schedule();
void update_state();

#endif
