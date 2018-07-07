#include "x86.h"
#include "device.h"


void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */

	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		case 1000:
			update_state();
			break;
		default:break;//assert(0);
	}
	current->regs = tf;
	schedule();	
}





void GProtectFaultHandle(struct TrapFrame *tf){
	panic("Here are some fault int Function GProtectFaultHandle");
	return;
}
