
#include "ARM_measurement_unit.h"

void performeMeasurements(uint8_t *);

//extern uint8_t measurement_results[SAMPLE_COUNT];

void initAdc0(void){
	/* Initilize ADC module timer*/
	if(SAMPLE_DELAY > 0){
		initialize_measurement_delay_timer(SAMPLE_DELAY);
	}

	/*ADC Module Initialization*/

	//1. Enable clock to ADC0
	SYSCTL->RCGCADC |= (1<<0);

	//2. Enable clock to appropiate GPIO, for now E and pin is 0.
	SYSCTL->RCGCGPIO |= (1<<4);
	GPIOE->DIR |= ~(1<<0);

	//3. Analogue input pin, choose with pin is input.
	GPIOE->AFSEL |= (1<<0);

	//4. Disable digital functions to corresponding analogue input pins.
	GPIOE->DEN |= ~(1<<0);

	//5. Disable analogue isolation circut. Must be disabled for all input pins.
	GPIOE->AMSEL |= (1<<0);

	/*Sample Sequencer Configuration*/

	/*	Ensure that the sample sequencer is disabled by clearing the corresponding ASENn bit in the
	 *		ADCACTSS register. Programming of the sample sequencers is allowed without having them
	 *		enabled. Disabling the sequencer during programming prevents erroneous execution if a trigger
	 *		event were to occur during the configuration process.
	 */
	ADC0->ACTSS |= (0<<3)|(0<<2)|(0<<1)|(0<<0);

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
	ADC0->ACTSS |= (1<<3);

	// Clear flags
	ADC0->ISC = (1<<3);

}

void inintSyncCableInput(void){
	SYSCTL->RCGCGPIO |= (1<<5);   // enable clock to PORTF
	SYSCTL->RCGCGPIO |= (1<<3);   // enable clock to PORTD

	// configure PORTF for LED output
	GPIOF->DIR |= 0b1110;         // make PORTF3, 2, 1 output for LEDs
	GPIOF->DEN |= 0b1110;         // make PORTF4-0 digital pins

	// configure PORTD6 for falling edge trigger interrupt
	// 0x40 -> 0100 0000
	GPIOD->DIR |= (0<<0);        // make PORTD6 input pin
	GPIOD->DEN |= (1<<0);         // make PORTD6 digital pin
	GPIOD->PUR |= (1<<0);			//Pull up resistor
	GPIOD->IS  |= (0<<0);        // make bit 4, 0 edge sensitive
	GPIOD->IBE |= (0<<0);        // trigger is controlled by IEV
	GPIOD->IEV |= (0<<0);        // falling edge trigger
	GPIOD->ICR = (1<<0);         // clear any prior interrupt
	GPIOD->IM  |= (1<<0);         // unmask interrupt

	// enable interrupt in NVIC and set priority to 6
	NVIC->IP[3] |= (6<<5);       // set interrupt priority to 6
	NVIC->ISER[0] |= (1<<3);    // enable IRQ3
}

unsigned long initAdc0GetResults(void){
	//Init SS3
	ADC0->PSSI = (1<<3);
	//Wait until conversion is done
	while((ADC0->RIS & (1<<3)) == 0){};
	//Return results from SS3 FIFO
	return ADC0->SSFIFO3 & 0xFFF;
}

void performeMeasurements(uint8_t results[]){
	setBlueLED();
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
	setLED();
}

void measurement_signal_generation(void){
	;
}

void measurement_measure(uint8_t measurement_results[]){
	if ((GET_MEASUREMENTS_SENT_BIT == 1) || OVERWRITE_OLD_RESULTS || (GET_MEASUREMENTS_PRESENT_BIT == 0)){
		performeMeasurements(measurement_results);
	}
}

void measurement_work(uint8_t measurement_results[]){
	measurement_signal_generation();
	measurement_measure(measurement_results);
}
