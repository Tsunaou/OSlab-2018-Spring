#include "device.h"
#include "x86.h"

void reset_sem(int i)
{
    Sem[i].used  = 0;
    Sem[i].value = 0;
    Sem[i].list  = NULL;
}

void init_sem()
{
    for(int i = 0;i < MAX_SEM_NUM; i++)
    {
        reset_sem(i);
    }
}
int new_sem()
{
    for(int i = 0;i < MAX_SEM_NUM; i++)
    {
        if(Sem[i].used == 0)
        {
            Sem[i].used = 1;
            return i;
        }
    }

    return -1;  //init failure
}

