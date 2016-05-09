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
#include <string.h>

void waitNextAction(void);
void resetMeasurementResults(void);

extern uint8_t measurement_results[SAMPLE_COUNT] = {0};

void syncCableInterupHandler(void){
	// clear the interrupt flag
	GPIOD->ICR = (1<<0);
	//Check if interrupt occurred during communication, if so, set flag up.
	setIntOccurredValue(CHECK_INT_DURING_COMMUNICATION);
	//Perform measurement work
	measurement_work(measurement_results);
}

void communicationTimeout(void){
	TIMER3->ICR = (1<<0);
	communication_timeout = 1;
}

void resetMeasurementResults(void){
	if (GET_MEASUREMENTS_SENT_BIT){
		//Reinit or clean measurement results
		memset(measurement_results, 0, SAMPLE_COUNT * sizeof(uint8_t));
		clearMeasurementsResultsPresentBit();
	}
}

void waitNextAction(){
	uint8_t package[PACKAGE_SIZE+1];
	while (1){
		// Read new pacakge, where first byte is ID or GLOBAL_ID
		readPackage(package);

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
					measurement_signal_generation();
					break;

				case START_WORK:
					//Signal generation and measureing
					measurement_work(measurement_results);
					break;

				case DELETE_RESULTS:
					//Delete measurement results
					resetMeasurementResults();
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
					measurement_signal_generation();
					break;

				case START_WORK:
					//Signal generation and measureing
					measurement_work(measurement_results);
					break;

				case DELETE_RESULTS:
					//Delete measurement results
					resetMeasurementResults();
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

void main(void){

	//Init LEDs for feedback.
	initializeLEDs();
	// Default interval is 1, when calling delay_timer, prived counter how many times to delay.
	initialize_delay_timer();

	//Init UART 5 for communication.
	initializeUART5();

	//Init PWM signal generator
	/* TODO: Implement PWM signal generator, configurable with width and duration (impulse count). */

	//Init analogue-digital-converter for measurements
	initAdc0();

	//Init syncronization cable input.
	inintSyncCableInput();

	//Init checker if interrupt occurred during communication
	interrupt_occurred = 0;

	//Inint state
	initState();

	//	Main program and logic
	waitNextAction();
}
