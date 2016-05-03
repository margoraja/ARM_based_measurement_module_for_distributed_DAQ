
#include "ARM_measurement_unit.h"

void setStateBit(short);
void clearStateBit(short);

void setBlueLED(void){
	GPIOF->DATA = (1<<2);
}

void setGreenLED(void){
	GPIOF->DATA = (1<<3);
}

void setRedLED(void){
	GPIOF->DATA = (1<<1);
}

void setLEDoff(void){
	GPIOF->DATA = ~((1<<1)|(1<<2)|(1<<3));
}

void setLED(void){
	if (!GET_MEASUREMENTS_SENT_BIT && GET_MEASUREMENTS_PRESENT_BIT){
		setRedLED();
	} else {
		//Anything else -> LED is OFF
		setLEDoff();
	}

}

void initializeLEDs(void){
	/* 		LED configuration
	 * Step 1. 	Enable clock to GPIOF.
	 *			PS! Previously configured clocks to port must be kept.'
	 *			0 disabled, 1 enabled
	 */
	SYSCTL->RCGCGPIO |= (1<<5);

	//Step 2.	Set LED pins as output. Red on PF1, Blue on PF2, Green on PF3.
	//			0 disabled, 1 enabled
	GPIOF->DIR = (1<<1)|(1<<2)|(1<<3);

	//Step 3.	Enable digital input for LED pins.
	//			0 disabled, 1 enabled
	GPIOF->DEN = (1<<1)|(1<<2)|(1<<3);

	//Step 4.	Disable (turn off) LEDs.
	//			Over-write current with these: invert(enable pin1 | enable pin2 | enable pin3)
	//			0 disabled, 1 enabled
	setLEDoff();
}

void setReadyToWorkBit(void){
	setStateBit(READY_TO_WORK_BIT);
}

void setMeasurementsResultsPresentBit(void){
	setStateBit(MEASUREMENTS_PRESENT_BIT);
	setLED();
}

void setMeasurementsSentBit(void){
	setStateBit(MEASUREMENTS_SENT_BIT);
	setLED();
}

void setInvalidPackageBit(void){
	setStateBit(INVALID_PACKAGE_BIT);
}

void setStateBit(short bit_number){
	state |= (1 << bit_number);
}

void clearReadyToWorkBit(void){
	clearStateBit(READY_TO_WORK_BIT);
}

void clearMeasurementsResultsPresentBit(void){
	clearStateBit(MEASUREMENTS_PRESENT_BIT);
	setLED();
}

void clearMeasurementsSentBit(void){
	clearStateBit(MEASUREMENTS_SENT_BIT);
	setLED();
}

void clearInvalidPackageBit(void){
	clearStateBit(INVALID_PACKAGE_BIT);
}

void clearStateBit(short bit_number){
	state &= ~(1 << bit_number);
}

void initState(void){
	state = 0;
	setReadyToWorkBit();
}

void initIntOccurredValue(void){
	interrupt_occurred = 0;
}

void setIntOccurredValue(short value){
	interrupt_occurred = value;
}
