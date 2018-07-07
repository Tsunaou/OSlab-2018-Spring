#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"

#define MAX_STACK_SIZE  4096
#define MAX_PCB_NUM     6
#define TIME_SLICE      8

#define USTACK_START    0x200000
#define IDLE_START      0x290000
#define PROCESS_MEMSZ   0x10000


struct ProcessTable {
    uint8_t stack[MAX_STACK_SIZE];  
    struct TrapFrame* tf;
    enum { RUNNABLE, RUNNING, DEAD, BLOCKED } state;
    enum { USED, FREE } free;
    int timeCount;
    uint32_t sleepTime;
    uint32_t pid;
    struct ProcessTable *next;
};


struct ProcessTable pcb[MAX_PCB_NUM];

struct ProcessTable *p_Free;
struct ProcessTable *p_Runnable;
struct ProcessTable *p_Blocked;
struct ProcessTable *p_cur;

struct ProcessTable* get_new_pcb(); //create a new pcb
int get_pid(struct ProcessTable *pcb_node); //get the id of a process
void insert_pcb(struct ProcessTable *pcb_node, struct ProcessTable **p_head);  //insert a processtable in the list of pcb
void pick_up_pcb(struct ProcessTable *pcb_node, struct ProcessTable **p_head);  //pick up a processtable in the list of pcb
void delete_pcb(struct ProcessTable *pcb_node, struct ProcessTable **p_head);  //delete a processtable in the list of pcb
void init_ptlb(struct ProcessTable *pcb_node);   //init a ProcessTable   
void init_pcb();    //init the Process Control Block
void schedule();    //schedule


#endif
