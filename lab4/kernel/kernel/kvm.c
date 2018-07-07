#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SECTSIZE 512
#define PT_LOAD     1       /* Loadable program segment */


void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,   &tss,    sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt));

	/*
	 * 初始化TSS
	 */
	tss.ss0 = KSEL(SEG_KDATA);
	tss.esp0 = (uint32_t)pcb[0].stack + MAX_STACK_SIZE;

	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/
	asm volatile(	
			"movw %%ax,%%ds\n\t" 
			"movw %%ax,%%es\n\t" 
			"movw %%ax,%%ss\n\t" 
			"movw %%ax,%%fs"
			::"a" (KSEL(SEG_KDATA)));

	lLdt(0);
	
}

void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	 */
	struct ProcessTable* p = get_new_pcb();
	p_Runnable = NULL;
	p_cur = p;
	
	asm volatile(	
			"movw %%ax,%%es\n\t" 
			"movw %%ax,%%fs\n\t" 
			"movw %%ax,%%ds"
			::"a" (KSEL(SEG_UDATA)));
	asm volatile(	"sti");				
	asm volatile(	"pushfl");			
	asm volatile(	"cli");				

	p->tf->ss = USEL(SEG_UDATA);
	p->tf->esp = USTACK_START + PROCESS_MEMSZ;
	p->tf->eflags = 0x202;
	p->tf->cs = USEL(SEG_UCODE);
	p->tf->eip = entry;
	p->state = RUNNING;
	p->timeCount = TIME_SLICE;
	p->sleepTime = 0;

	asm volatile(	"pushl %0":: "r"(p->tf->ss));	//push %ss	
	asm volatile(	"pushl %0":: "r"(p->tf->esp));	//push %esp	
	asm volatile(	"pushfl");			//push eflags	
	asm volatile(	"pushl %0":: "r"(p->tf->cs));	//push cs
	asm volatile(	"pushl %0":: "r"(entry));	//push eip	
	asm volatile(	"iret");
	/*
	 * iret:
	 * pop eip
	 * pop CS
	 * pop eflags
	 */

}

void loadUMain(void) {

	/*加载用户程序至内存*/
	struct ELFHeader *elf;
	struct ProgramHeader *ph,*eph;

	unsigned char buf[10000];

	for(int i = 1; i <= 200; i++)
	{
		void* dst = (void*)(buf + (i-1)*SECTSIZE); 
		readSect(dst, i + 200);
	}

	elf = (struct ELFHeader *)buf;
	/* Load each program segment */
	ph = (void*)elf + elf->phoff;
	eph = ph + elf->phnum;
	
	for(; ph < eph; ph++) {
		if (ph->type == PT_LOAD) { 

			/* TODO: copy the segment from the ELF file to its proper memory area */
			//mymemcpy((void*)ph->vaddr,(void*)ph->off,ph->filesz);
			unsigned int dst = ph->vaddr;
			unsigned int src = ph->off;
			unsigned int nums = ph->filesz ;
			
			while (nums--)
			 {
				*(unsigned char*)dst = *(unsigned char*)(buf + src);
				dst++;
				src++;
			}

			/* TODO: zeror the memory area [vaddr + file_sz, vaddr + mem_sz) */
			if(ph->filesz<ph->memsz)
			{
				//mymemset((void*)(ph->vaddr+ph->filesz),0,(ph->memsz-ph->filesz));
				nums = ph->memsz - ph->filesz;
				while(nums--)
				{
						*(unsigned char*)dst = 0;
						dst++;
				}
			}
			
		}
	}

	uint32_t entry = (uint32_t)(elf->entry);
	//entry();
	enterUserSpace(entry);

}
