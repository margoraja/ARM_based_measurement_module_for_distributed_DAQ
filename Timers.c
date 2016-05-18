
#include "ARM_measurement_unit.h"

#define PRESCALE_VALUE 159

void delay_timer_init(void){
	// Enable GPTM timer 0
	setBit(&(SYSCTL->RCGCTIMER), 0, 1 );

	// Disable timer A for configuration
	setBit(&(TIMER0->CTL), 0, 0 );

	// Configure timer0 to be two independent 16 bit timers (required for prescaler)
	TIMER0->CFG = 0x4;

	// Set timer counting direction
	setBit(&(TIMER0->TAMR), 0, 1 );

	// Set prescale value
	TIMER0->TAPR = PRESCALE_VALUE;
}

void delay_timer(uint16_t delayInMilliSeconds){
	// Calculate the counter reload value
	uint32_t timedInterval = delayInMilliSeconds * 0.001 * SYSTEM_CLOCK;
	uint32_t counterReloadValue = (timedInterval/(PRESCALE_VALUE+1)) - 1;

	// Configure counter reload value
	TIMER0->TAILR = counterReloadValue;

	// Enable TimerA
	setBit(&(TIMER0->CTL), 0, 1 );

	// Loops until timer event has elapsed
	while (1)
	{
		// Check to see if timer event has elapsed
		if (getBit(TIMER0->CTL, 0 ) == 0)
		{
			break;
		}
	}
}

void initialize_communication_timeout_timer(void){
	//timer
	SYSCTL->RCGCTIMER |= (1<<3);
	//timer off
	TIMER3->CTL &= ~(1<<0);
	//GPTM register
	TIMER3->CFG = 0x00000000;
	//TnMR
	TIMER3->TAMR |= (0x1<<0);
	//set count direction
	TIMER3->TAMR &= ~(1<<4);
	//count to
	TIMER3->TAILR = COMMUNICATION_TIMEOUT;
	//enable interupt
	TIMER3->IMR = (1<<0);
	//Clear interrupt at NVIC
	NVIC->ICPR[1] |= (1<<3);
	//Enable interrupt on timer 3.
	NVIC->ISER[1] |= (1<<3);
}

void initialize_measurement_delay_timer(int delay){
	//Enable timer.
	SYSCTL->RCGCTIMER |= (1<<4);
	//GPTM register
	TIMER4->CTL &= ~(1<<0);
	//GPTM register
	TIMER4->CFG = 0x00000000;
	//Using period mode.
	TIMER4->TAMR |= (0x2<<0);
	//Set count direction from up to down.
	TIMER4->TAMR |= (0<<4);
	//Counter from which to count to / from which to count from.
	TIMER4->TAILR = delay;
	//Activate timer.
	TIMER4->CTL |= (1<<0);
}

void enabale_communication_timeout_timer(void){
	TIMER3->CTL |= (1<<0);
	TIMER3->ICR |= (1<<0);
}

void disabale_communication_timeout_timer(void){
	TIMER3->CTL |= (0<<0);

}

void reset_communication_timeout_timer(void){
	disabale_communication_timeout_timer();
	enabale_communication_timeout_timer();
}

int get_communication_timeout_flag(void){
	return (TIMER3->RIS & 0x00000001);
}

void measurement_delay(void){
	TIMER4->ICR |= (1<<0);
	while(1){
		//check if counted down
		if ((TIMER4->RIS & 0x00000001) == 1) {
			break;
		}
	}
}
