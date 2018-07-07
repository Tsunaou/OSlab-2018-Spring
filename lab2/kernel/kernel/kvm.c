#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;
record pc_rec;

#define SECTSIZE 512
#define PT_LOAD     1       /* Loadable program segment */


void readSeg(unsigned char *pa, int len, int offset);


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
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt));

	pc_rec.index = SEG_TSS + 1;
	pc_rec.base = 0x400000;

	tss.esp0 = 0x200000;
	tss.ss0 = KSEL(SEG_KDATA);
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	asm volatile("movw %%ax,%%es":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ds":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ss":: "a" (KSEL(SEG_KDATA)));

	lLdt(0);
	
}


void loadUMain(void) {

	/*加载用户程序至内存*/
	struct ELFHeader *elf;
	struct ProgramHeader *ph,*eph;

	char *buf = (void*)0x400000;

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

	initProcess(elf->entry);
}

void readSeg(unsigned char *pa, int offset, int len) {
	unsigned char *epa = pa + len;
	pa -= offset & SECTSIZE;
	offset = offset / SECTSIZE + 201;
	for(; pa < epa; offset++, pa += SECTSIZE)
		readSect(pa, offset);
}