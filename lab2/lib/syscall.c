#include "lib.h"
#include "types.h"
#include <stdarg.h>

/*/*
 * io lib here
 * 库函数写在这
 */
#define SYS_sleep 162
#define SYS_exit 1
#define SYS_fork 2
#define SYS_write 4     // defined in <sys/syscall.h>
#define BUFFER_SIZE 1000    // max size of buffer
  
static char Buf[BUFFER_SIZE];
static uint32_t index = 0;

void Push_Ch(char ch);
void Push_Str(char *str);
void Push_Dec(int dec);
void Push_Hex(uint32_t hex);


static inline int32_t syscall(int num, uint32_t a1,uint32_t a2, uint32_t a3)
{
	int32_t ret = 0;
	asm volatile("int $0x80": "=a"(ret) : "a"(num), "b"(a1), "c"(a2), "d"(a3));

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
    syscall(SYS_write,1,(uint32_t)Buf,lenth);
    
}

int fork() {
	return syscall(SYS_fork, 0, 0, 0);
}

void sleep(int st) {
	syscall(SYS_sleep, st, 0, 0);
}

void exit() {
	syscall(SYS_exit, 0, 0, 0);
}