#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;

void IDLE()
{
	asm volatile("movl %0, %%esp" : : "i"(IDLE_START));
	asm volatile("sti");
	waitForInterrupt();
}

struct ProcessTable* get_new_pcb()
{
	int index = 0;
	if (p_Free != NULL)
	{
		index = p_Free->pid;
		p_Free = p_Free->next;
	}
	struct ProcessTable *p = &pcb[index - 1];
	p->free = USED;
	p->next = NULL;
	p->sleepTime = 0;
	p->state = RUNNABLE;
	p->timeCount = TIME_SLICE;

	insert_pcb(p, &p_Runnable);
	return p;
}

int get_pid(struct ProcessTable *pcb_node)
{
	return 1 + (pcb_node - pcb);
}


void insert_pcb(struct ProcessTable *pcb_node, struct ProcessTable **p_head)
{
	if (*p_head == NULL)
		*p_head = pcb_node;
	else
	{
		struct ProcessTable *p = *p_head;
		while (p->next != NULL)
			p = p->next;
		p->next = pcb_node;
	}
}

void pick_up_pcb(struct ProcessTable *pcb_node, struct ProcessTable **p_head)
{
	if (pcb_node == *p_head)//要删的是头结点
	{
		*p_head = (*p_head)->next;
		pcb_node->next = NULL;
	}
	else
	{
		struct ProcessTable *p = *p_head;
		struct ProcessTable *q = (*p_head)->next;
		while (q != NULL)
		{
			if (q->pid == pcb_node->pid)
			{
				p->next = q->next;
				pcb_node->next = NULL;
				break;
			}
			p = q;
			q = q->next;
		}
	}
}

void delete_pcb(struct ProcessTable *pcb_node, struct ProcessTable **p_head)
{
	pick_up_pcb(pcb_node, p_head);
	init_ptlb(pcb_node);
}

void init_ptlb(struct ProcessTable *pcb_node)
{
	pcb_node->tf->edi = 0;   pcb_node->tf->esi = 0;
	pcb_node->tf->ebp = 0;   pcb_node->tf->xxx = 0;
	pcb_node->tf->ebx = 0;   pcb_node->tf->edx = 0;
	pcb_node->tf->ecx = 0;   pcb_node->tf->eax = 0;

	pcb_node->tf->irq = 0;   pcb_node->tf->error = 0;

	pcb_node->tf->eip = 0;
	pcb_node->tf->eflags = 0x202;
	pcb_node->tf->esp = 0x200000;

	pcb_node->state = DEAD;
	pcb_node->timeCount = 0;
	pcb_node->sleepTime = 0;
	pcb_node->free = FREE;
	pcb_node->next = NULL;
}

void init_pcb()
{
	for (int i = 0; i < MAX_PCB_NUM; i++)
	{
		struct ProcessTable *pcb_node = &pcb[i];
		mymemcpy((void*)pcb_node->stack, 0, MAX_STACK_SIZE);
		pcb[i].tf = (struct TrapFrame *)((uint32_t)pcb[i].stack + MAX_STACK_SIZE - sizeof(struct TrapFrame));
		init_ptlb(pcb_node);
		pcb[i].pid = i + 1;


		if (i == (MAX_PCB_NUM - 1))
			pcb[i].next = NULL;
		else
			pcb[i].next = &pcb[i + 1];
	}

	p_cur = NULL;
	p_Free = &pcb[0];
	p_Runnable = NULL;
	p_Blocked = NULL;

}

void schedule()
{
	if(p_cur == NULL && p_Runnable == NULL)
	{
		//putChar('@');
		/*if(p_Blocked!=NULL)
		{
			//int times = p_Blocked->sleepTime;
			//if(times % 30 == 0)
				//printbug("SleepTime = %d\n",times);
			//putChar('1');
		}*/
		IDLE();
	}
	else
	{
		//putChar('s');
		p_cur = p_Runnable;
		p_Runnable = p_Runnable->next;
		tss.esp0 = (uint32_t)p_cur->stack + MAX_STACK_SIZE;
		tss.ss0  = KSEL(SEG_KDATA);

		gdt[SEG_UCODE] = SEG(STA_X | STA_R, (p_cur->pid - 1) * PROCESS_MEMSZ, 0xffffffff, DPL_USER);
		gdt[SEG_UDATA] = SEG(STA_W,         (p_cur->pid - 1) * PROCESS_MEMSZ, 0xffffffff, DPL_USER);
		asm volatile("pushl %eax");
		asm volatile("movl %0, %%eax" : : "r"(USEL(SEG_UDATA)));
		asm volatile("movw %ax, %ds");
		asm volatile("movw %ax, %es");
		asm volatile("popl %eax");


		asm volatile("movl %0, %%esp" : : "r"(p_cur->stack + MAX_STACK_SIZE - sizeof(struct TrapFrame)));
		asm volatile("popl %gs");
		asm volatile("popl %fs");
		asm volatile("popl %es");
		asm volatile("popl %ds");
		asm volatile("popal");
		asm volatile("addl $4, %esp");
		asm volatile("addl $4, %esp");
		asm volatile("iret");

	}
	
	//printbug("Here in schedule last\n");

}    