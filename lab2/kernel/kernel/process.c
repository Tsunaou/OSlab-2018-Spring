#include "x86.h"
#include "device.h"

PCB *current, proc0;
PCB pcb_pool[MAX_PCB_SIZE];
list_head runnable, blocked, free;
list_head *next;
SegDesc gdt[NR_SEGMENTS];
TSS tss;

uint8_t schedule_flag;
static int last_pid;

LIST_HEAD(runnable);
LIST_HEAD(blocked);
LIST_HEAD(free);

void idle();

void initProcess(uint32_t entry) {
	current = NULL;

	last_pid = 2;

	int i = 0;
	for (; i < MAX_PCB_SIZE; i++) {
		pcb_pool[i].state = FREE;
		list_add(&pcb_pool[i].state_list, &free);
	}

	proc0.regs = (struct TrapFrame *)((uint32_t)proc0.kstack + MAX_STACK_SIZE - sizeof(struct TrapFrame));

	void (*p)();
	p = idle;

	proc0.regs->es = KSEL(SEG_KDATA);
	proc0.regs->cs = KSEL(SEG_KCODE);
	proc0.regs->ss = KSEL(SEG_KDATA);
	proc0.regs->ds = KSEL(SEG_KDATA);
	proc0.regs->eflags = 0x202;
	proc0.regs->esp = 0x100000;
	proc0.regs->eip = (uint32_t)*p;

	schedule_flag = 1;

	PCB *proc1 = new_pcb();
	proc1->regs->es = USEL(SEG_UDATA);
	proc1->regs->cs = USEL(SEG_UCODE);
	proc1->regs->ss = USEL(SEG_UDATA);
	proc1->regs->ds = USEL(SEG_UDATA);
	proc1->regs->eflags = 0x202;
	proc1->regs->esp = 0x200000;
	proc1->regs->eip = entry;
	proc1->pid = 0;
}

PCB *new_pcb() {
	pcb_pool[last_pid].state = RUNNABLE;
	list_del(&pcb_pool[last_pid].state_list);
	list_add(&pcb_pool[last_pid].state_list, &runnable);
	pcb_pool[last_pid].regs = (struct TrapFrame *)((uint32_t)pcb_pool[last_pid].kstack + MAX_STACK_SIZE - sizeof(struct TrapFrame));
	return &pcb_pool[last_pid++];

	return NULL;
}

void schedule() {
	if (schedule_flag == 0) return;
	if (!list_empty(&runnable) && ((current == NULL) || (current == &proc0))) {
		list_head *temp = &runnable;
		temp = temp->next;
		current = list_entry(temp, PCB, state_list);
		current->state = RUNNING;
		current->timeCount = TIMECOUNT;
		tss.esp0 = (uint32_t)current->kstack + MAX_STACK_SIZE;
		schedule_flag = 0;
	}
	else {
		current = &proc0;
		schedule_flag = 1;
	}
}

void update_state() {
	int i = 0;
	for (; i < MAX_PCB_SIZE; i++) {
		if (pcb_pool[i].state == RUNNING) {
			pcb_pool[i].timeCount--;
			if (pcb_pool[i].timeCount == 0) {
				pcb_pool[i].state = RUNNABLE;
				schedule_flag = 1;
			}
		}
		else if (pcb_pool[i].state == BLOCKED) {
			pcb_pool[i].sleepTime--;
			if (pcb_pool[i].sleepTime == 0) {
				pcb_pool[i].state = RUNNABLE;
				list_del(&pcb_pool[i].state_list);
				list_add(&pcb_pool[i].state_list, &runnable);
			}
		}
	}
}