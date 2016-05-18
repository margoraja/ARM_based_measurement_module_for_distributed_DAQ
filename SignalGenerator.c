
#include "ARM_measurement_unit.h"

#define MAX_LOAD 65535 //Maximum value that PWM LOAD can hold is 16bit.

void initializePWM(void){

	// Enable clock to PWM module
	//SetBit(&(SYSCTL->RCGC0), 20, 1 );
	SYSCTL->RCGC0 |= (1<<20);

	// Enable clock to GPIOB
	//SetBit(&(SYSCTL->RCGCGPIO), 1, 1 );
	SYSCTL->RCGCGPIO |= (1<<1);
	// Set PB6 as output
	//SetBit(&(GPIOB->DIR), 6, 1);
	GPIOB->DIR |= (1<<6);
	// Set PB6 as digital function
	//SetBit(&(GPIOB->DEN), 6, 1);
	GPIOB->DEN |= (1<<6);
	// Set PB6 to use alternate function
	//SetBit(&(GPIOB->AFSEL), 6, 1 );
	GPIOB->AFSEL |= (1<<6);
	// Configure PB6 to use M0PWM0 as alternate function
	//GPIOB->PCTL |= (0x4 << 24);
	GPIOB->PCTL |= (0x4<<24);

	if (USE_CLOCK_DIVIDER){
		// Configure to use clock divider
		//SetBit(&(SYSCTL->RCC), 20, 1 );
		SYSCTL->RCC |= (1<<20);
		// Set divider value
		if (CLOCK_DIVIDER<8){
			SYSCTL->RCC |= (CLOCK_DIVIDER << 17);
		} else {while(1);} //Invalid clock divider value.
	}

	// Disable PWM0 for configuration
	PWM0->_0_CTL = 0x0;

	//Disable interrupts on PWM0
	PWM0->_0_INTEN = 0x0;
	PWM0->INVERT = 0x0;

	// Configure PWM A.
	PWM0->_0_GENA = 0x8C;

	// Set load
	if (PWM_LOAD > MAX_LOAD){while(1);} // Invalid load value.
	PWM0->_0_LOAD = PWM_LOAD;

	// Set width
	if (PWM_WIDTH > PWM_LOAD){while(1);}  //Invalid width value.
	PWM0->_0_CMPA = PWM_WIDTH;
}

void enablePWM(void){
	//Start PWM timer
	if ((PWM0->_0_CTL & (1<<0)) != 1){
		PWM0->_0_CTL = 0x1;
	}
	//Enable PWM, enable M0PWM0 output
	if ((PWM0->ENABLE & (1<<0)) != 1){
		PWM0->ENABLE = 0x1;
	}
}

void disablePWM(void){
	//Disable PWM, disable M0PWM0 output
	if ((PWM0->ENABLE & (1<<0)) != 0){
		PWM0->ENABLE = 0x0;
	}
	//Stop PWM timer
	if ((PWM0->_0_CTL & (1<<0)) != 0){
		PWM0->_0_CTL = 0x0;
	}
	//Clear timer counter
	SYSCTL->SRPWM = (1<<0);
}
