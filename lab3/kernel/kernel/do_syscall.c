#include "x86.h"
#include "device.h"
#include "common.h"

#define GS 0xb8000
#define PRINT_START (80 * 5 + 0) * 2

static short *site = (short*)(GS + PRINT_START);

static int cow = 5;
static int col = 0;

extern void putChar(char ch);
void Video_Print(int row, int col, char ch);
void Screen_Print(char ch);


void Video_Print(int row, int col, char ch)
{
	site = (short*)(GS + (80*row + col) * 2);
	short ax = (0x0c << 8) + (short)ch;							
	*site = ax;
}


void Screen_Print(char ch)
{
	if(ch =='\n')
	{
		cow++;
		col=0;
	}
	else
	{
		Video_Print(cow, col, ch);
		col++;
		if(col == 80)
		{
			cow++;
			col=0;
		}
	}
}




void sys_write(struct TrapFrame *tf) 
{
	
	if(tf->eax == SYS_write && (tf->ebx == 1 || tf->ebx == 2))
	{
		char *buf = (char*)tf->ecx + (p_cur - pcb) * PROCESS_MEMSZ;
		int n = tf->edx;
		char ch = 0;
		

		for(int i = 0; i < n ;i++)
		{
			ch = *(buf++);

			if(ch == '\0')
				break;

			Screen_Print(ch);
			putChar(ch);

			tf->error++;
		}

		if(tf->error == n) //if ok , return lenth of string
			tf->eax = tf->edx;
		else
		{
			tf->eax = -1; //return -1
		}
	}
	else
	{
		panic("There is something wrong in Function sys_write\n");
	}
		
}

void sys_fork(struct TrapFrame *tf)
{
	struct ProcessTable *child  = get_new_pcb();
	struct ProcessTable *father = p_cur;

	int src = USTACK_START + (father->pid - 1) * PROCESS_MEMSZ;
	int dst = USTACK_START + (child->pid - 1) * PROCESS_MEMSZ;

	mymemcpy((void*)dst, (void*)src, PROCESS_MEMSZ);
	mymemcpy((void*)child->stack, (void*)father->stack, MAX_STACK_SIZE);

	child->tf->eax = 0;
	father->tf->eax = child->pid;
	
	father->state = RUNNABLE;

	//printbug("Here in sys_fork\n");

	//father->next = NULL;
	//insert_pcb(father, &p_Runnable);
	p_Runnable = father;
	father->next = child;
	p_cur = NULL;

	schedule();
} 

void sys_sleep(uint32_t time)
{
	p_cur->sleepTime = time;
	p_cur->state = BLOCKED;
	struct ProcessTable* q = p_cur;
	q->next = NULL;
	insert_pcb(q, &p_Blocked);
	p_cur = NULL;
	schedule();
}

void sys_exit(struct TrapFrame *tf)
{
	struct ProcessTable *q = p_cur;
	init_ptlb(q);
	q->next = NULL;
	insert_pcb(q,&p_Free);
	p_cur = NULL;
	schedule();
}  

