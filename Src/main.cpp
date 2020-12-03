#include "../kernel/arikeos.hpp"

using namespace std;

#define BUS_FREQ 16000000
#define QUANTA   5 // Run each thread for 5millsec

uint32_t led_task_counter;
uint32_t dsp_task_counter;
uint32_t diagnostic_task_counter;

void led_task(void);
void dsp_task(void);
void diagnostic_task(void);

int main()
{
	ArikeOS sysOS(BUS_FREQ, QUANTA);
	// Add threads
	sysOS.osKernelAddThreads(&led_task, &dsp_task, &diagnostic_task);
	// Initialize the kernel
	sysOS.osKernelInit();
	// Launch
	sysOS.osKernelLaunch();
}

void led_task(void){
	// Thread runs until time quanta is up
	while(1){
		// Do some computation
		led_task_counter++;
	}
}

void dsp_task(void){
	while(1){
		// Do some computation
		dsp_task_counter++;
	}
}

void diagnostic_task(void){
	while(1){
		diagnostic_task_counter++;
	}
}
