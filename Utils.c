
#include "ARM_measurement_unit.h"

void blueLedOn(void){
	SetBit(&GPIOF->DATA, 2, 1);
}

void greenLedOn(void){
	SetBit(&GPIOF->DATA, 3, 1);
}

void redLedOn(void){
	SetBit(&GPIOF->DATA, 1, 1);
}

void LedOff(void){
	SetBit(&GPIOF->DATA, 1, 0);
	SetBit(&GPIOF->DATA, 2, 0);
	SetBit(&GPIOF->DATA, 3, 0);
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
	SetBit(&(SYSCTL->RCGCGPIO), 5, 1);

	//Step 2.	Set LED pins as output. Red on PF1, Blue on PF2, Green on PF3.
	//			0 disabled, 1 enabled
	SetBit(&(GPIOF->DIR), 1, 7);

	//Step 3.	Enable digital input for LED pins.
	//			0 disabled, 1 enabled
	SetBit(&(GPIOF->DEN), 1, 7);

	//Step 4.	Disable (turn off) LEDs.
	//			Over-write current with these: invert(enable pin1 | enable pin2 | enable pin3)
	//			0 disabled, 1 enabled
	LedOff();
}

void setReadyToWorkBit(void){
	SetBit(&state, READY_TO_WORK_BIT, 1);
}

void setMeasurementsResultsPresentBit(void){
	SetBit(&state, MEASUREMENTS_PRESENT_BIT, 1);
	setLed();
}

void setMeasurementsSentBit(void){
	SetBit(&state, MEASUREMENTS_SENT_BIT, 1);
	setLed();
}

void setInvalidPackageBit(void){
	SetBit(&state, INVALID_PACKAGE_BIT, 1);
}
/*
void setStateBit(uint8_t bit_number){
	state |= (1 << bit_number);
}*/

void clearReadyToWorkBit(void){
	SetBit(&state, READY_TO_WORK_BIT, 0);
}

void clearMeasurementsResultsPresentBit(void){
	SetBit(&state, MEASUREMENTS_PRESENT_BIT, 0);
	setLed();
}

void clearMeasurementsSentBit(void){
	SetBit(&state, MEASUREMENTS_SENT_BIT, 0);
	setLed();
}

void clearInvalidPackageBit(void){
	SetBit(&state, INVALID_PACKAGE_BIT, 0);
}
/*
void clearStateBit(uint8_t bit_number){
	state &= ~(1 << bit_number);
}*/

void initState(void){
	state = 0;
	setReadyToWorkBit();
}

void initIntOccurredValue(void){
	interrupt_occurred = 0;
}

void setIntOccurredValue(uint8_t value){
	interrupt_occurred = value;
}

uint8_t GetBit(volatile uint32_t target, uint8_t position){
	return ((target & (0x01 << position)) != 0);
}

void SetBit(volatile uint32_t* target, uint8_t position, uint8_t bit){
	bit ? ((*target) |= (0x01 << position)) : ((*target) &= ~(0x01 << position));
}
