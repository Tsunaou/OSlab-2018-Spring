#include "x86.h"
#include "device.h"
#include "common.h"

#include <stdarg.h>


#define BUFFER_SIZE 1000    // max size of buffer
  
static char Buf[BUFFER_SIZE];
static int index = 0;

void Push_Ch(char ch);
void Push_Str(char *str);
void Push_Dec(int dec);
void Push_Hex(uint32_t hex);



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

void printbug(const char *str, ...)
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
	int ind = 0;
	while(ind != index)
	{
		putChar(Buf[ind]);
		ind++;
	}
	
}