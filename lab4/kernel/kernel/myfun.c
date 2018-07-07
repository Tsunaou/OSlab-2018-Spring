#include "x86.h"
#include "device.h"
#include "common.h"

void* mymemcpy (void *destination, const void* source, unsigned int num )
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
}
