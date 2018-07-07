#include "x86.h"
#include "device.h"
#include "common.h"

void syscallHandle(struct TrapFrame *tf);
void GProtectFaultHandle(struct TrapFrame *tf);
void TimerInteruptHandle(struct TrapFrame *tf);
void schedule();
void sys_write(struct TrapFrame *tf);
void sys_fork(struct TrapFrame *tf);
void sys_sleep(uint32_t time);
void sys_exit(struct TrapFrame *tf);
void sys_sem_init(struct TrapFrame *tf);
void sys_sem_post(struct TrapFrame *tf);
void sys_sem_wait(struct TrapFrame *tf);
void sys_sem_destroy(struct TrapFrame *tf);



void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序	
	 */
	/* Reassign segment register */

	asm volatile(	
			"movw %%ax,%%es\n\t" 
			"movw %%ax,%%fs\n\t" 
			"movw %%ax,%%ds"
			::"a" (KSEL(SEG_KDATA)));
	//asm volatile("cli");
	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		case 0x20:
			TimerInteruptHandle(tf);
			break;	
		default:
		{
			printbug("IrqHandle tf->irq = %d\n",tf->irq);
			assert(0);
		};
	}
	//asm volatile("sti");

}

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	//printbug("In Func syscallHandle, tf->eax= %d\n",tf->eax);
	switch(tf->eax) {
		case 0: 
			enableInterrupt();
			disableInterrupt();
			panic("add_irq_handle in Function syscallHandle\n");
			break;
		case SYS_write:
			//printbug("SYS_write\n"); 
			sys_write(tf); 
			break;
		case SYS_exit:
			//printbug("SYS_exit\n"); 
			sys_exit(tf);
			break;
		case SYS_fork:
			//printbug("SYS_fork\n"); 
			sys_fork(tf);
			break;
		case SYS_nanosleep:
			//printbug("SYS_nanosleep\n"); 
			sys_sleep(tf->ebx);
			break;
		case SYS_sem_init:
			sys_sem_init(tf);
			break;
		case SYS_sem_post:
			sys_sem_post(tf);
			break;
		case SYS_sem_wait:
			sys_sem_wait(tf);
			break;
		case SYS_sem_destroy:
			sys_sem_destroy(tf);
			break;
		default: panic("Here are some fault int Function syscallHandle");
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	panic("Here are some fault int Function GProtectFaultHandle");
	return;
}

void TimerInteruptHandle(struct TrapFrame *tf)
{
	//printbug("TimerInt!\n");
	struct ProcessTable *p = p_Blocked;

	while(p != NULL)
	{
		if(p->sleepTime > 0)
			p->sleepTime--;
		p = p->next;
		//int times = p->sleepTime;
		//printbug("%d",times);
	}

	p = p_Blocked;
	if(p != NULL)
	{
		//putChar('x');
		//int times = p->sleepTime;
		//printbug("SleepTime = %d\n",times);
		if(p->sleepTime == 0)
		{	
			p_Blocked = p_Blocked->next;
			p->next = NULL;
			//p->timeCount = TIME_SLICE;
			insert_pcb(p,&p_Runnable);
		}
		p = p_Blocked;
	}

	if(p_cur == NULL)
	{
		//putChar('n');
		schedule();
		return;
	}

	p_cur->timeCount--;
	if(p_cur->timeCount == 0)
	{
		p_cur->state = RUNNABLE;
		p_cur->timeCount = TIME_SLICE;
		p = p_cur;
		p->next = NULL;
		insert_pcb(p, &p_Runnable);
		p_cur = NULL;
		schedule();
	}

	return;
}
