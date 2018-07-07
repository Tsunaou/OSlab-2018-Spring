#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SYS_sleep 162
#define SYS_exit 1
#define SYS_fork 2
#define SYS_write 4

#define GS 0xb8000
#define PRINT_START (80 * 5 + 0) * 2

static short *site = (short*)(GS + PRINT_START);
static int cow = 5;
static int col = 0;

extern void putChar(char ch);
void Video_Print(int row, int col, char ch);
void Screen_Print(char ch);

uint32_t get_base(uint32_t index) {
	return gdt[index].base_15_0 + (gdt[index].base_23_16 << 16) + (gdt[index].base_31_24 << 24);
}

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
		char *buf = (void *)tf->ecx + get_base(tf->ds >> 3);
		int n = tf->edx;
		char ch = 0;
		int error = 0; // counts for lenth of string in *buf
		

		for(int i = 0; i < n ;i++)
		{
			ch = *(buf++);

			if(ch == '\0')
				break;

			Screen_Print(ch);
			putChar(ch);

			error++;
		}

		if(error == n) //if ok , return lenth of string
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

void sys_fork(struct TrapFrame *tf) {
	uint32_t *father_start = (uint32_t *)get_base(tf->ds >> 3);
	uint32_t *father_end = (uint32_t *)father_start + 0x200000;
	uint32_t *child_start = (uint32_t *)pc_rec.base;
	while (father_start < father_end) {
		*child_start = *father_start;
		father_start++;
		child_start++;
	}

	PCB *child = new_pcb();

	int i = 0;
	for (; i < MAX_STACK_SIZE; i++)
		child->kstack[i] = current->kstack[i];
	tf->eax = child - pcb_pool;

	uint32_t seg_code = pc_rec.index++;
	uint32_t seg_data = pc_rec.index++;
	gdt[seg_code] = SEG(STA_X | STA_R, (uint32_t)pc_rec.base, 0x200000, DPL_USER);
	gdt[seg_data] = SEG(STA_W, (uint32_t)pc_rec.base, 0x200000, DPL_USER);
	pc_rec.base += 0x200000;
	child->regs->es = USEL(seg_data);
	child->regs->cs = USEL(seg_code);
	child->regs->ss = USEL(seg_data);
	child->regs->ds = USEL(seg_data);

	// if father is idle, return child's pid, else return 0
	child->regs->eax = 0;
}

void sys_sleep(uint32_t st) {
	current->state = BLOCKED;
	next = current->state_list.next;
	list_del(&current->state_list);
	list_add(&current->state_list, &blocked);
	current->sleepTime = st;
	schedule_flag = 1;
}

void sys_exit() {
	current->state = FREE;
	next = current->state_list.next;
	list_del(&current->state_list);
	list_add(&current->state_list, &free);
	schedule_flag = 1;
}

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	switch(tf->eax) {
		case 0: 
			enableInterrupt();
			disableInterrupt();
			panic("add_irq_handle in Function syscallHandle\n");
			break;
		case SYS_write: 
			sys_write(tf); 
			break;
		case SYS_fork:
			sys_fork(tf);
			break;
		case SYS_sleep:
			sys_sleep(tf->edx);
			break;
		case SYS_exit:
			sys_exit(tf);
			break;
		default: panic("Here are some fault int Function syscallHandle");
	}
}