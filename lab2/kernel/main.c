#include "common.h"
#include "x86.h"
#include "device.h"

void idle(void) {
	while (1) waitForInterrupt();
}

void kEntry(void) {

	initSerial();// initialize serial port
	initIdt(); // initialize idt
	initIntr(); // iniialize 8259a
	initSeg(); // initialize gdt, tss
	initTimer();
	loadUMain(); // load user program, enter user space

	enableInterrupt();
	idle();
	assert(0);
}
