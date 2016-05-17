
#include "ARM_measurement_unit.h"

void performeMeasurements(uint8_t *);

void initAdc0(void){
	/* Initilize ADC module timer*/
	if(SAMPLE_DELAY > 0){
		initialize_measurement_delay_timer(SAMPLE_DELAY);
	}

	/*ADC Module Initialization*/

	/* TODO: Implement a way to decreace ADC clock speed,
	 * 			so that ADC produced more accurate measurements
	 * 			but with the cost of slower sample rate.
	 */
	//1. Enable clock to ADC0
	SetBit(&(SYSCTL->RCGCADC), 0, 1);

	//2. Enable clock to appropiate GPIO, for now E and pin is 0.
	SetBit(&(SYSCTL->RCGCGPIO), 4, 1);
	SetBit(&(GPIOE->DIR), 0, 0);

	//3. Analogue input pin, choose with pin is input.
	SetBit(&(GPIOE->AFSEL), 0, 1);

	//4. Disable digital functions to corresponding analogue input pins.
	SetBit(&(GPIOE->DEN), 0, 0);

	//5. Disable analogue isolation circut. Must be disabled for all input pins.
	SetBit(&(GPIOE->AMSEL), 0, 1);

	/*Sample Sequencer Configuration*/

	/*	Ensure that the sample sequencers are disabled by clearing the corresponding ASENn bit in the
	 *		ADCACTSS register. Programming of the sample sequencers is allowed without having them
	 *		enabled. Disabling the sequencer during programming prevents erroneous execution if a trigger
	 *		event were to occur during the configuration process.
	 */
	SetBit(&(ADC0->ACTSS), 0, 0);
	SetBit(&(ADC0->ACTSS), 1, 0);
	SetBit(&(ADC0->ACTSS), 2, 0);
	SetBit(&(ADC0->ACTSS), 3, 0);

	/*	2. Configure the trigger event for the sample sequencer in the ADCEMUX register.
	 *		Must add gpio pin and internal solution in the end.
	 */
	//External GPIO is 0x4. See pages 834 & 655
	//Using SW trigger
	ADC0->EMUX = (0xF<<12);

	/*	3. When using a PWM generator as the trigger source, use the ADC Trigger Source Select
	 *		(ADCTSSEL) register to specify in which PWM module the generator is located. The default
	 *		register reset selects PWM module 0 for all generators.
	 */

	/* 	4. For each sample in the sample sequence, configure the corresponding input source in the
	 * 		ADCSSMUXn register.
	 * 		Using PE0 -> AIN3 where ain3 is analog input 3 which is PE0
	 */
	ADC0->SSMUX3 |= 3;

	/*	5. For each sample in the sample sequence, configure the sample control bits in the corresponding
	 * 		nibble in the ADCSSCTLn register. When programming the last nibble, ensure that the END bit
	 * 		is set. Failure to set the END bit causes unpredictable behavior.
	 */
	ADC0->SSCTL3 |= 0x6;

	/*	6. If interrupts are used, set the corresponding MASK bit in the ADCIM register
	 */
	//ADC0->IM = (1<<3);	//A sample has competed conversion and the respective ADCSSCTL3 IEn bit is set, enabling a raw interrupt.

	/*	7. Enable the sample sequencer logic by setting the corresponding ASENn bit in the ADCACTSS register.
	 */
	SetBit(&(ADC0->ACTSS), 3, 1);

	// Clear flags
	SetBit(&(ADC0->ISC), 3, 1);

}

void inintSyncCableInput(void){
	SYSCTL->RCGCGPIO |= (1<<3);   // enable clock to PORTD

	// configure PORTD6 for falling edge trigger interrupt
	// 0x40 -> 0100 0000
	SetBit(&(GPIOD->DIR), 0, 0);		// make PORTD6 input pin
	SetBit(&(GPIOD->DEN), 0, 1);		// make PORTD6 digital pin
	SetBit(&(GPIOD->PUR), 0, 1);		// Pull up resistor
	SetBit(&(GPIOD->IS), 0, 0);			// make bit 4, 0 edge sensitive
	SetBit(&(GPIOD->IBE), 0, 0);		// trigger is controlled by IEV
	SetBit(&(GPIOD->IEV), 0, 0);		// falling edge trigger
	SetBit(&(GPIOD->ICR), 0, 1);		// clear any prior interrupt
	SetBit(&(GPIOD->IM), 0, 1);			// unmask interrupt

	// enable interrupt in NVIC and set priority to 6
	NVIC->IP[3] |= (6<<5);       		// set interrupt priority to 6
	SetBit(&(NVIC->ISER[0]), 3, 1);     // enable Interrupts on GPIO D
}

unsigned long initAdc0GetResults(void){
	//Init SS3
	SetBit(&(ADC0->PSSI), 3, 1);
	//Wait until conversion is done
	while((ADC0->RIS & (1<<3)) == 0){};
	//Return results from SS3 FIFO
	return ADC0->SSFIFO3 & 0xFFF;
}

void performeMeasurements(uint8_t results[]){
	blueLedOn();
	unsigned int counter = 0;
	/*	Must be able to set "internval" or sample rate. -> Porbably set in ARM_measurement_unit.h file.*/
	while (SAMPLE_COUNT > counter){
		if(SAMPLE_DELAY > 0){
			measurement_delay();
		}
		results[counter] = initAdc0GetResults();
		counter++;
	}
	setMeasurementsResultsPresentBit();
	clearMeasurementsSentBit();
	setLed();
}

void measurement_measure(uint8_t measurement_results[]){
	if ((GET_MEASUREMENTS_SENT_BIT == 1) || OVERWRITE_OLD_RESULTS || (GET_MEASUREMENTS_PRESENT_BIT == 0)){
		performeMeasurements(measurement_results);
	}
}
