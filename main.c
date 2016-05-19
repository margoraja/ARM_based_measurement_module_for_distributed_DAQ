/*
 * When using "=" or "&=", it will overwrite everything else that is configued.
 * When using "|=", it wil change speciffic bit accordingly and keep the rest.
 * When configuring GPIOn, it is wise to use "|=" instead of "=" and "&=" because
 * 		it wont overwrite already configured configurations.
 * 		E.g. configuring UART5 (using PE4 and PE5) and ADC0 (to use PE0) in different
 * 		functions requries using "|=", otherwise confgurations will be overwritten.
 *
 */

#include "ARM_measurement_unit.h"
#include "Communication.h"
#include "Measurement.h"
#include "SignalGenerator.h"

void waitNextAction(void);
void clearMeasurementResults(void);
void measurementPwmAndAdc(uint8_t *);
void inintSyncWireInterupt(void);
void enableGpiodInterrupts(void);
void disableGpiodInterrupts(void);

extern uint8_t measurement_results[SAMPLE_COUNT] = {0};

void main(void){
	byte_counter = 0;

	//Inint state
	initState();

	//Init LEDs for feedback.
	initializeLEDs();

	// Default interval is 1, when calling delay_timer, prived counter how many times to delay.
	delay_timer_init();

	//Init PWM signal generator
	//initializePWM();

	//Init analogue-digital-converter for measurements
	initAdc0();

	//Init checker if interrupt occurred during communication
	interrupt_occurred = 0;

	//Init UART 5 for communication.
	initializeUART5();

	//Init syncronization cable input.
	inintSyncWireInterupt();

	//Set status to ready to work.
	setReadyToWorkBit();

	//enablePWM();

	//	Main program and logic
	waitNextAction();
}

void waitNextAction(){
	uint8_t package[PACKAGE_SIZE+1];
	while (1){
		enableGpiodInterrupts();
		// Read new pacakge, where first byte is ID or GLOBAL_ID
		readPackage(package);
		disableGpiodInterrupts();

		// Byte did not contain ID.
		if (package[0] == ID){
			switch(package[1]){
				case SEND_RESULTS:
					// Send measurement results.
					sendResults(measurement_results);
					break;

				case START_MEASURING:
					//Start measurements.
					measurement_measure(measurement_results);
					break;

				case START_SIGNAL:
					//Start signal generation.
					enablePWM();
					break;

				case STOP_SIGNAL:
					disablePWM();
					break;

				case START_WORK:
					//Signal generation and measureing
					measurementPwmAndAdc(measurement_results);
					break;

				case DELETE_RESULTS:
					//Delete measurement results
					clearMeasurementResults();
					break;

				case GET_STATE:
					sendState();
					break;

				default:
					// Something non-implemented is sent, must be ignored.
					break;
			}
		} else if (package[0] == GLOBAL_ID){
			switch(package[1]){

				case START_MEASURING:
					//Start measurements.
					measurement_measure(measurement_results);
					break;

				case START_SIGNAL:
					//Start signal generation.
					enablePWM();
					break;

				case STOP_SIGNAL:
					disablePWM();
					break;

				case START_WORK:
					//Signal generation and measureing
					measurementPwmAndAdc(measurement_results);
					break;

				case DELETE_RESULTS:
					//Delete measurement results
					clearMeasurementResults();
					break;

				case SEND_RESULTS:
					// "SEND_RESULTS" received with global id is not possible.
					//		Otehrwise all devices in network will start to send measurement results.
					break;

				case GET_STATE:
					// "GET_STATE" received with global id is not possible.
					//		Otehrwise all devices in network will start to send state.
					break;

				default:
					// Something non-implemented is sent, must be ignored.
					break;
			}
		}
		// Set invalid package bit to OK.
		if (GET_INVALID_PACKAGE_BIT == 1){clearInvalidPackageBit();}
	}
}

void clearMeasurementResults(void){
	if (GET_MEASUREMENTS_SENT_BIT){
		//Reinit or clean measurement results
		memset(measurement_results, 0, SAMPLE_COUNT * sizeof(uint8_t));
		clearMeasurementsResultsPresentBit();
	}
}

void measurementPwmAndAdc(uint8_t measurement_results[]){
	//enablePWM();
	measurement_measure(measurement_results);
	//disablePWM();
}

void inintSyncWireInterupt(void){
	SYSCTL->RCGCGPIO |= (1<<3);   // enable clock to PORTD

	// configure PORTD6 for falling edge trigger interrupt
	setBit(&(GPIOD->DIR), 0, 0);		// make PORTD6 input pin
	setBit(&(GPIOD->DEN), 0, 1);		// make PORTD6 digital pin
	setBit(&(GPIOD->PUR), 0, 1);		// Pull up resistor
	setBit(&(GPIOD->IS), 0, 0);			// make bit 4, 0 edge sensitive
	setBit(&(GPIOD->IBE), 0, 0);		// trigger is controlled by IEV
	setBit(&(GPIOD->IEV), 0, 0);		// falling edge trigger
	setBit(&(GPIOD->ICR), 0, 1);		// clear any prior interrupt
	setBit(&(GPIOD->IM), 0, 1);			// unmask interrupt

	// enable interrupt in NVIC and set priority to 6
	NVIC->IP[3] |= (6<<5);       		// Set interrupt priority to 6
	setBit(&(NVIC->ICPR[0]), 3, 1);		// Clear pending interrupts
	enableGpiodInterrupts();
}

void enableGpiodInterrupts(void){
	if (!getBit(NVIC->ISER[0], 3)){
		setBit(&(NVIC->ISER[0]), 3, 1);     // enable Interrupts on GPIO D
	}
}

void disableGpiodInterrupts(void){
	if (getBit(NVIC->ISER[0], 3)){
		setBit(&(NVIC->ISER[0]), 3, 0);     // disable Interrupts on GPIO D
	}
}

void communicationTimeoutInterupHandler(void){
	//Only when timeotut was reached.
	if (getBit(TIMER3->RIS, 0)){
		communication_timeout = 1;
	}
	//Clear the interrupt flag
	TIMER3->ICR = (1<<0);
	//Clear any pending interrupts in register
	NVIC->ICPR[1] |= (1<<3);
}

void syncCableInterupHandler(void){
	//Perform measurement work only if interrupt was received from PD1
	if (GET_READY_TO_WORK_BIT && getBit(GPIOD->RIS, 0)){
		//Perform measurements
		measurementPwmAndAdc(measurement_results);
		//Check if interrupt occurred during communication, if so, set flag up.
		setIntOccurredValue(CHECK_INT_DURING_COMMUNICATION);
	}
	//Clear the interrupt flag
	GPIOD->ICR = (1<<0);
	//Clear any pending interrupts in register
	NVIC->ICPR[0] |= (1<<3);
}
