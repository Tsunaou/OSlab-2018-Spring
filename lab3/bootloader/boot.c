#include "boot.h"

#define SECTSIZE 512
#define PT_LOAD  1       /* Loadable program segment */


void bootMain(void) {
	/* 加载内核至内存，并跳转执行 */

	struct ELFHeader *elf;
	struct ProgramHeader *ph,*eph;

	char *buf = (char*)0x200000;

	for(int i = 1; i <= 200; i++)//for sector 1 to 200
	{
		void* dst = (void*)(buf + (i-1)*SECTSIZE); 
		readSect(dst, i);
	}

	elf = (void*)buf;

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
			//if(ph->filesz<ph->memsz)
			//{
				//mymemset((void*)(ph->vaddr+ph->filesz),0,(ph->memsz-ph->filesz));
				/*nums = ph->memsz - ph->filesz;
				while(nums--)
				{
						*(unsigned char*)dst = 0;
						dst++;
				}*/
			//
			
		}
	}

	void (*entry)(void);
	entry = (void*)(elf->entry);
	entry();

}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
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


/*void* mymemcpy (void *destination, const void* source, unsigned int num )
{
 	char *temp = (char *) destination, *s = (char *) source;  
  
    while (num--)  
        *temp++ = *s++;  
  
    return destination;  
}

void* mymemset (void *ptr, int value, unsigned int num )
{
 	char *temp = (char *) ptr;  
    while (num--)  
        *temp++ = value;  

    return ptr;  
}*/
