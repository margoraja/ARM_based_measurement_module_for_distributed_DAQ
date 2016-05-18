
#include "ARM_measurement_unit.h"

void blueLedOn(void){
	GPIOF->DATA = (1<<2);
}

void greenLedOn(void){
	GPIOF->DATA = (1<<3);
}

void redLedOn(void){
	GPIOF->DATA = (1<<1);
}

void LedOff(void){
	GPIOF->DATA = 0x0;
}

void setLed(void){
	if (!GET_MEASUREMENTS_SENT_BIT && GET_MEASUREMENTS_PRESENT_BIT){
		redLedOn();
	} else {
		//Anything else -> LED is OFF
		LedOff();
	}
}

void initializeLEDs(void){
	/* 		LED configuration
	 * Step 1. 	Enable clock to GPIOF.
	 *			PS! Previously configured clocks to port must be kept.'
	 *			0 disabled, 1 enabled
	 */
	setBit(&(SYSCTL->RCGCGPIO), 5, 1);

	//Step 2.	Set LED pins as output. Red on PF1, Blue on PF2, Green on PF3.
	//			0 disabled, 1 enabled
	setBit(&(GPIOF->DIR), 1, 7);

	//Step 3.	Enable digital input for LED pins.
	//			0 disabled, 1 enabled
	setBit(&(GPIOF->DEN), 1, 7);

	//Step 4.	Disable (turn off) LEDs.
	//			Over-write current with these: invert(enable pin1 | enable pin2 | enable pin3)
	//			0 disabled, 1 enabled
	LedOff();
}

void setReadyToWorkBit(void){
	setBit(&state, READY_TO_WORK_BIT, 1);
}

void setMeasurementsResultsPresentBit(void){
	setBit(&state, MEASUREMENTS_PRESENT_BIT, 1);
	setLed();
}

void setMeasurementsSentBit(void){
	setBit(&state, MEASUREMENTS_SENT_BIT, 1);
	setLed();
}

void setInvalidPackageBit(void){
	setBit(&state, INVALID_PACKAGE_BIT, 1);
}
/*
void setStateBit(uint8_t bit_number){
	state |= (1 << bit_number);
}*/

void clearReadyToWorkBit(void){
	setBit(&state, READY_TO_WORK_BIT, 0);
}

void clearMeasurementsResultsPresentBit(void){
	setBit(&state, MEASUREMENTS_PRESENT_BIT, 0);
	setLed();
}

void clearMeasurementsSentBit(void){
	setBit(&state, MEASUREMENTS_SENT_BIT, 0);
	setLed();
}

void clearInvalidPackageBit(void){
	setBit(&state, INVALID_PACKAGE_BIT, 0);
}
/*
void clearStateBit(uint8_t bit_number){
	state &= ~(1 << bit_number);
}*/

void initState(void){
	state = 0;
}

void initIntOccurredValue(void){
	interrupt_occurred = 0;
}

void setIntOccurredValue(uint8_t value){
	interrupt_occurred = value;
}

uint8_t getBit(volatile uint32_t target, uint8_t position){
	return ((target & (0x01 << position)) != 0);
}

void setBit(volatile uint32_t* target, uint8_t position, uint8_t bit){
	bit ? ((*target) |= (0x01 << position)) : ((*target) &= ~(0x01 << position));
}
