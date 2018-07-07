#include "lib.h"
#include "types.h"
#include <stdarg.h>
#include "../kernel/include/common/const.h"



/*/*
 * io lib here
 * 库函数写在这
 */

#define BUFFER_SIZE 1000    // max size of buffer
  
static char Buf[BUFFER_SIZE];
static uint32_t index = 0;

void Push_Ch(char ch);
void Push_Str(char *str);
void Push_Dec(int dec);
void Push_Hex(uint32_t hex);


int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;

	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/

	asm volatile("int $0x80":"=a"(ret)	//eax
				:"a"(num) 	//eax
				,"b"(a1)	//ebx
				,"c"(a2)	//ecx
				,"d"(a3)	//edx
				,"D"(a4)	//edi
				,"S"(a5));	//esi
		
	return ret;
}

void Push_Str(char *str)
{
	while(*str)
	{
		Push_Ch(*str++);
	}
}


void Push_Ch(char ch)
{
	Buf[index] = ch;
	index++;
}


void Push_Dec(int dec)
{
	if(dec == 0)
		return;
	else if(dec < 0)
	{
		Push_Ch('-');
		dec = -dec;
		Push_Dec(dec);
		return;
	}
	else
	{
		Push_Dec(dec/10);
		Push_Ch((char)(dec % 10 + '0'));
	}
}


void Push_Hex(uint32_t hex)
{
	if(hex == 0)
		return;
	
	Push_Hex(hex/16);

	hex = hex%16;

	if(hex < 10)
		Push_Ch((char)(hex%16 + '0'));
	else
		Push_Ch((char)(hex%16 - 10 + 'a')); 
}


void printf(const char *str, ...)
{
	//double varg_float = 0;
	int varg_int = 0;
	uint32_t varg_hex = 0;
	char* varg_str = 0;
	char varg_ch = 0;
	const char* pstr = 0;
	
	va_list vp;
	va_start(vp, str);

	pstr = str;

	while(*pstr)
	{
		if(*pstr == '%')
		{
			switch(*(++pstr))
			{
				case 'c':
					varg_ch = va_arg(vp, int);
					Push_Ch(varg_ch);	
					break;
				case 'd':
					varg_int = va_arg(vp, int);
					if(varg_int == 0)
						Push_Ch('0');
					else if(varg_int == 0x80000000)
					{
						Push_Str("-2147483648");
					}
					else
						Push_Dec(varg_int); 
					break;
				case 's':
					varg_str = va_arg(vp, char*);
					Push_Str(varg_str);
					break;
				case 'x':
					varg_hex = va_arg(vp, uint32_t);
					if(varg_hex == 0)
						Push_Ch('0');
					else
						Push_Hex(varg_hex);
					break;
				default:
					break;
			
			}
			pstr++;
		}
		else
		{
			Push_Ch(*pstr++);
		}
	}
	va_end(vp);
	
	Buf[index]='\0';
	uint32_t lenth = index;
	index = 0;
	syscall(SYS_write,1,(uint32_t)Buf,lenth,0,0);
	
}

int fork()
{
	return syscall(SYS_fork, 0, 0, 0, 0, 0);
	//printf("I am in fork !\n");    
}

void sleep(uint32_t time)
{
	syscall(SYS_nanosleep, time, 0, 0, 0, 0);
	//printf("I am in sleep !\n");  
}

int exit()
{
	return syscall(SYS_exit, 0, 0, 0, 0, 0);
	//printf("I am in exit !\n");  
}

int sem_init(sem_t *sem, uint32_t value)
{
	return syscall(SYS_sem_init, (uint32_t)sem, value, 0, 0 ,0);
}

int sem_post(sem_t *sem)
{
	return syscall(SYS_sem_post, (uint32_t)sem, 0, 0, 0 ,0);	
}

int sem_wait(sem_t *sem)
{
	return syscall(SYS_sem_wait, (uint32_t)sem, 0, 0, 0 ,0);
}

int sem_destroy(sem_t *sem)
{
	return syscall(SYS_sem_destroy, (uint32_t)sem, 0, 0, 0 ,0);
}


