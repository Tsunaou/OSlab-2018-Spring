#----|-----entry------|-errorcode-|-----id-----|---handler---|
.globl vec0;    vec0:   pushl $0;  pushl    $0; jmp asmDoIrq
.globl vec1;    vec1:   pushl $0;  pushl    $1; jmp asmDoIrq
.globl vec3;    vec3:   pushl $0;  pushl    $3; jmp asmDoIrq
.globl vec4;    vec4:   pushl $0;  pushl    $4; jmp asmDoIrq
.globl vec5;    vec5:   pushl $0;  pushl    $5; jmp asmDoIrq
.globl vec6;    vec6:   pushl $0;  pushl    $6; jmp asmDoIrq
.globl vec7;    vec7:   pushl $0;  pushl    $7; jmp asmDoIrq
.globl vec8;    vec8:              pushl    $8; jmp asmDoIrq
.globl vec9;    vec9:   pushl $0;  pushl    $9; jmp asmDoIrq
.globl vec10;   vec10:             pushl   $10; jmp asmDoIrq
.globl vec11;   vec11:             pushl   $11; jmp asmDoIrq
.globl vec12;   vec12:             pushl   $12; jmp asmDoIrq
.global irqGProtectFault
	irqGProtectFault:			   pushl   $13;	jmp asmDoIrq
.globl vec14;   vec14:             pushl   $14; jmp asmDoIrq
.globl vec16;   vec16:  pushl $0;  pushl   $16; jmp asmDoIrq

.global irqEmpty
		irqEmpty:		pushl $0;	pushl $-1;	jmp asmDoIrq

.global irqSyscall;	irqSyscall:	pushl $0x80;	jmp asmDoIrq

.globl irq0; irq0: pushl $1000; jmp asmDoIrq
.globl irq1; irq1: pushl $1001; jmp asmDoIrq
.globl irq14; irq14: pushl $1014; jmp asmDoIrq

# ！调用C编写的中断处理程�?见irq/irqHandle.c)�?
.globl asmDoIrq
.extern irqHandle
.extern current
asmDoIrq:
	pushal
        
	pushl %ds
	pushl %es
	pushl %esp
	movl $0x10, %eax
	movl %eax, %ds
	movl %eax, %es
	call irqHandle
	#addl $4, %esp
	movl (current), %eax
	movl (%eax), %esp
	popl %es
	popl %ds

	popal
	addl $4, %esp
	
	iret
