#include "arikeos.hpp"
#include "mcal_reg.hpp"
#include "../../Drivers/CMSIS/Include/cmsis_gcc.h"


using namespace std;

#define STACKSIZE 100    // 100 32-bit values or words
#define NUM_OF_THREADS 3

extern "C" void osSchedulerLaunch(void);

// Thread control block - using Linkedlist
struct tcb{
	int32_t *stackPtr;   /* Pointer to stack */
	struct tcb *nextPtr; /* Pointer to next thread */
};

typedef struct tcb tcbType;
tcbType *currentThread;

// Stack for all threads in the OS
int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];

tcbType tcbs[NUM_OF_THREADS];


ArikeOS::ArikeOS(){}

ArikeOS::ArikeOS(uint32_t bus_frequency, uint32_t rrquanta){
	this->bus_frequency = bus_frequency;
	this->round_robin_quanta = rrquanta;
}


void ArikeOS::osKernelStackInit(uint32_t i){
	// i = thread index
	// Stack Pointer. STACKSIZE - 16 = position of stack pointer
	tcbs[i].stackPtr = &TCB_STACK[i][STACKSIZE - 16];
	TCB_STACK[i][STACKSIZE - 1] = 0x01000000; // Set thumb bit to 1 in PSR register
	/*
	 * Program counter (PC) register is located in
	 * TCB_STACK[i][STACKSIZE - 2]
	 */
	/* Registers saved automatically by processor */
	TCB_STACK[i][STACKSIZE - 3] = 0x14141414; // R14
	TCB_STACK[i][STACKSIZE - 4] = 0x12121212; // R12
	TCB_STACK[i][STACKSIZE - 5] = 0x03030303; // R3
	TCB_STACK[i][STACKSIZE - 6] = 0x02020202; // R2
	TCB_STACK[i][STACKSIZE - 7] = 0x01010101; // R1
	TCB_STACK[i][STACKSIZE - 8] = 0x03030303; // R0

	/* Registers saved manually */
	TCB_STACK[i][STACKSIZE - 9] = 0x11111111;  // R11
	TCB_STACK[i][STACKSIZE - 10] = 0x10101010; // R10
	TCB_STACK[i][STACKSIZE - 11] = 0x09090909; // R9
	TCB_STACK[i][STACKSIZE - 12] = 0x08080808; // R8
	TCB_STACK[i][STACKSIZE - 13] = 0x07070707; // R7
	TCB_STACK[i][STACKSIZE - 14] = 0x06060606; // R6
	TCB_STACK[i][STACKSIZE - 15] = 0x05050505; // R5
	TCB_STACK[i][STACKSIZE - 16] = 0x04040404; // R4
}

osflag_type ArikeOS::osKernelAddThreads(function_type task0, function_type task1,function_type task2){
	// Disable interrupt globally
	__disable_irq();

	tcbs[0].nextPtr =  &tcbs[1]; /* Thread 1 should run after  Thread 0 */
	tcbs[1].nextPtr = &tcbs[2];  /* Thread 2 should run after  Thread 1 */
	tcbs[2].nextPtr = &tcbs[0];  /* Thread 0 should run after  Thread 2 */

	osKernelStackInit(0);        /* Initialize the stack for Thread 0   */
	TCB_STACK[0][STACKSIZE - 2] = (int32_t)(task0); /* Point the PC to task function task0 */

	osKernelStackInit(1);        /* Initialize the stack for Thread 1   */
	TCB_STACK[1][STACKSIZE - 2] = (int32_t)(task1); /* Point the PC to task function task1 */

	osKernelStackInit(2);        /* Initialize the stack for Thread 2   */
	TCB_STACK[2][STACKSIZE - 2] = (int32_t)(task2); /* Point the PC to task function task2 */

	// Let currentThread points to tcb0
	currentThread = &tcbs[0];

	// Enable interrupt
	__enable_irq();

	return 1;
}

void ArikeOS::osKernelInit(void){
	// bus_frequency = 16MHz (different for other MCU)
	this->millis_prescaler = (this->bus_frequency / 1000); //  millisec
}

void ArikeOS::osKernelLaunch(void){
	// Disable systick before configuring it.
	mcal::reg::reg_access<uint32_t,
				uint32_t,
				mcal::reg::systick_ctrl,
				0x00>::reg_set();

	// Reset Current VAL reg
	mcal::reg::reg_access<uint32_t,
				uint32_t,
				mcal::reg::systick_val,
				0x00>::reg_set();

	// Specify time to system timer to generate
	// interrupt and switch to another thread.
	// Write time quanta to the systick register
	// The systick interrrupt will occur after every time value
	// passed to it. Subtract 1 bcuz we count from zero
	SysTick->LOAD = (this->round_robin_quanta * this->millis_prescaler) - 1;
	SYSPRI3 = (SYSPRI3&0x00FFFFFF) | 0xE0000000; // Priority 7

	// Enable Systick
	mcal::reg::reg_access<uint32_t,
				uint32_t,
				mcal::reg::systick_ctrl,
				0x07>::reg_set();

	// Launch the scheduler from Assembler
	osSchedulerLaunch();
}

void ArikeOS::setOS_sysBusFrequency(uint32_t freq){
	this->bus_frequency = freq;
}

void ArikeOS::setOS_rrQuanta(uint32_t quanta){
	this->round_robin_quanta = quanta;
}

uint32_t ArikeOS::getOS_sysBusFrequency() const{
	return this->bus_frequency;
}

uint32_t ArikeOS::getOS_rrQuanta() const{
	return this->round_robin_quanta;
}

uint32_t ArikeOS::getOS_millisPrescaler() const{
	return this->millis_prescaler;
}

ArikeOS::~ArikeOS()
{
	// Do some clean up.
}
