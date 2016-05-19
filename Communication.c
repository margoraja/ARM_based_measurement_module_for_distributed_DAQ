
#include "ARM_measurement_unit.h"
#include "CRC.h"

void initializeRS485Controller(void);
void setCommunicationToTransmit(void);
void setCommunicationToReceive(void);

uint8_t readFirstByte(void);
uint8_t readNextBytes(void);

void writePackageOut(uint8_t *, uint8_t);
void __writeByteOut(uint8_t);


/*	Communication protocol uses three bytes on both ways of communication:
 * 		1: Device ID. NULL for sending to all devices.
 * 		2: Action. What must be performed.
 * 		3: Extra data or info. NULL for nothing
 * 	One exception, when sending measurement data to controller, X bytes are sent wher X is total count:
 * 		1: Device ID who sends.
 * 		2-3: In addition N*2 bytes measurement ID.
 * 		4-(N*3 or N*4): Measurement results. N*2 bytes if used > 8bit ADC, otherwise N bytes.
 * 		X-1 and X: CRC
 */

void setCommunicationToTransmit(void){
	/* 1. Disable UART5
	 * 2. Reconfigure RS485 to transmit
	 * 3. Enable UART5 with transmitting
	 */
	greenLedOn();
	delay_timer(UART_SWITCHING_DELAY);
	UART5->CTL = 0x0;
	setBit(&(GPIOB->DATA), 4, ~(RS485_RECEIVE));
	setBit(&(UART5->CTL), 0, 1);
	setBit(&(UART5->CTL), 8, 1);
}

void setCommunicationToReceive(void){
	/* First of all wait just a little to ensure that everything is sent before disabling UART.
	 * 1. Disable UART5
	 * 2. Reconfigure RS485 to receive
	 * 3. Enable UARt5 with receive
	 */
	delay_timer(UART_SWITCHING_DELAY);
	UART5->CTL = 0x0;
	setBit(&(GPIOB->DATA), 4, RS485_RECEIVE);
	setBit(&(UART5->CTL), 0, 1);
	setBit(&(UART5->CTL), 9, 1);
	setLed();
}

void initializeRS485Controller(void){
	//Enable clock to PBx GPIO block.
	setBit(&(SYSCTL->RCGCGPIO), 1, 1);
	//Enable PB4 port control for controlling RS485 receive and transmit.
	setBit(&(GPIOB->PCTL), 16, 1);
	//Disable analogue mode.
	setBit(&(GPIOB->AMSEL), 4, 0);
	//Enable PB4 as direction output
	setBit(&(GPIOB->DIR), 4, 1);
	//Disable Alternate functions
	setBit(&(GPIOB->AFSEL), 4, 0);
	//Enable digital
	setBit(&(GPIOB->DEN), 4, 1);
}

void initializeUART5(void){
	// Initialize timer5 for changing RS485 mode.

	//Initlize RS485 chip controller. Sets to listening.
	initializeRS485Controller();

	/*		UART and PIN configuration
	 *Step 1. 	Enables UART5 module, PE4-PE5. Page 344
	 *			0 disabled, 1 enabled
	 */
	setBit(&(SYSCTL->RCGCUART), 5, 1);

	/*
	 * Step 2. 	Enables clock to appropiate GPIO. Required to set it to E, which is 0. Page 340.
	 *			There for enable clock to GPIOE.
	 *			0 disabled, 1 enabled
	 */
	setBit(&(SYSCTL->RCGCGPIO), 4, 1);
	//Step 3. 	Enable required pins to GPIO AFSEL. Must set pins for GPIOE, required are PE4 and PE5 (4 and 5).
	//			0 controlled by register, 1 controller by alternate hardware function.
	setBit(&(GPIOE->AFSEL), 4, 1);
	setBit(&(GPIOE->AFSEL), 5, 1);

	/* Step 4.	Current level and/or slew rate are not required.*/

	/* Step 5. 	Assigning UART signals to specific pins. PMCx Bit Field Encoding for PE4-PE5 is 1.
	 * 			PMC0-7 represent x in PAx. Eg 4 and 5 for PE4 and PE5.
	 *			PMC0 starts with 0 and PMC1 stastrs with 4. There for 1 must be set into 16 (PMC4)and 20 (PMC5) locations.
	 *			0 disabled, 1 enabled
	 */
	setBit(&(GPIOE->PCTL), 16, 1);
	setBit(&(GPIOE->PCTL), 20, 1);

	//			Since UART5 is digital signal, enable DigitalENable signals for specific PINs.
	//			0 disabled, 1 enabled
	setBit(&(GPIOE->DEN), 4, 1);
	setBit(&(GPIOE->DEN), 5, 1);

	//			Disabe analogue functions
	setBit(&(GPIOE->AMSEL), 4, 1);
	setBit(&(GPIOE->AMSEL), 5, 1);

	/*		Communikation clocks, baud-rate.
	 *		Using default clock which is 16MHz.
	 *		Required baud-rate for communication is 115200.
	 *		Integer BRD = System frequency/(16*115200) = 8,...-> 8
	 *		Fraction BRD = (BRD - integer(BRD))*64 + 0,5 = 44,... -> 44
	 * 		115200: iBRD 8, fBRD 44
	 */

	//Step 1.	Disable UART by clearing the UARTEN bit in the UARTCTL register
	UART5->CTL = 0x0;

	//Step 2.	Set integer portion of the baud-rate(BRD). Integer BRD = IBRD.
	UART5->IBRD = INTEGER_BRD;

	//Step 3.	Fraction portion of the baud-rate(BRD). Fraction BRD = FBRD.
	UART5->FBRD = FLOAT_BRD;

	//Step 4. 	Set serial parameters UARTLCRH (UART line control).
	UART5->LCRH = (0x3<<5);

	//Step 5. 	Configure UART clock source.
	//			Using system clock.
	UART5->CC = 0x0;

	/*Step -.	Micro Direct Memory Access.
	 *			Might be required for sending data to controller. Read from memory and send to controller.
	 *
	 *Step 6.	Enable UART once more.
	 *			Since Receive and Transmit requires UARTTEN pin bit enabling as well then
	 * 			all three must be enable at the same time (UARTTEN bit 0, TXE bit 8 and RXE bit 9).
	 *			0 disabled, 1 enabled
	 */
	setCommunicationToReceive();
	initialize_communication_timeout_timer();
}

/*
 * 	##### Reading data from UART <- RS485 #####
 */

uint8_t readFirstByte(void){
	/* Used to read first byte of package
	 * Wait for UART5 not to be busy (full).
	 *		Read UAR-FR bit 4 -> 0 or 1 -> compare to != 0.
	 *		0 is not empty, 1 is empty.
	 *	Read data from UART5 Data (8 bits) and return it.
	 */
	while((UART5->FR & (1<<4)) != 0);
	return UART5->DR;
}

uint8_t readNextBytes(void){
	/* Used to read next bytes, not the first one.
	 * Waits untill A) Byte is received, B) Interrupt was called -> package is discarded, C) New byte was not received brefore timeout.
	 */
	while(((UART5->FR & (1<<4)) != 0) && (!interrupt_occurred) && (communication_timeout == 0));
	return UART5->DR;
}

void readPackage(uint8_t *package){
	int all_not_Ok = 1;
	/* Running untill all is OK:
	 * 		No interrupts during communication
	 * 		No timeout during communication
	 * 		CRC is OK
	 * 		first byte is ID or Global ID
	 */
	while(all_not_Ok){
		/* Reset byte counter. */
		byte_counter = 0x0;
		/* Reset interrupt occurred value. */
		initIntOccurredValue();
		/* Reset communication timeout value. */
		communication_timeout = 0x0;
		/* Clear package. */
		memset(package, 0x0, PACKAGE_SIZE * sizeof(uint8_t));
		/* Read in the first byte. */
		package[byte_counter] = readFirstByte();
		/* Increment byte counter because first byte has been read in. */
		byte_counter++;
		/* Enable communication timeout timer. */
		enabale_communication_timeout_timer();
		/* Read pacakge */
		while ((byte_counter < PACKAGE_SIZE) && (interrupt_occurred == 0) && (communication_timeout == 0)){
			/* Reset communication timeout timer. */
			reset_communication_timeout_timer();
			/* Read next byte. */
			package[byte_counter] = readNextBytes();
			byte_counter++;
		}
		/* Discard data when not valid:
		 * 			Interrupt occurred during package read in
		 * 			First byte is not ID nor Global ID
		 * 			Timeout occurred during package read in
		 * 			Last byte (CRC) and calcualted CRC do not match
		 */
		if ((!interrupt_occurred) &&
				(package[0] == ID || package[0] == GLOBAL_ID) &&
				!communication_timeout){
			if (CRC_INCLUDED && !(checkCrcIsOk(package))){
				setInvalidPackageBit();
			} else {
				all_not_Ok = 0; /* All good, received valid package. */
			}
		}
		/* Disable communication timeout timer as it is not needed any more. */
		disabale_communication_timeout_timer();
	}
	//Make sure that byte_counter is 0 when not reading data from UART.
	byte_counter = 0;
}

/*
 * 	##### Writing data to UART -> RS485 #####
 */

void sendResults(uint8_t results[]){
	if (GET_MEASUREMENTS_PRESENT_BIT){
		int counter = 0;
		uint8_t package[PACKAGE_SIZE];
		package[0] = 107;
		setCommunicationToTransmit();
		while (SAMPLE_COUNT >= counter){
			/*
			 * results[counter] & 0xFC0; //ADC result bits 0-6, mask: 0b000000111111 = 63 = 3F
			 * plus data start mask (0b01******)
			 */
			package[1] = DATA_START|(results[counter]>>6);
			/*
			 * ADC result bits 7-11, mask: 0b111111000000 = 4032 = FC0
			 * plus data end mask (0b11******)
			 */
			package[2] = DATA_END|results[counter] & 0x3F;
			/*
			 * CRC calculated from package[0] to package[2]
			 */
			package[3] = calculate_CRC(package);

			writePackageOut(package, 0);
			counter++;

			//Slight delay before sending new pacakge.
			//Might require more precise timer and not that long delay.
			delay_timer(1);
		}
		setCommunicationToReceive();
		setMeasurementsSentBit();
	}
}

void sendState(void){
	uint8_t package[PACKAGE_SIZE];
	package[0] = ID;
	setCommunicationToTransmit();
	package[1] = GET_STATE;
	package[2] = state;
	package[3] = calculate_CRC(package);
	writePackageOut(package, 0);
	setCommunicationToReceive();
	clearInvalidPackageBit();
}

void writePackageOut(uint8_t *package, uint8_t change_rs485_mode){
	int i;
	if (change_rs485_mode){
		setCommunicationToTransmit();
	}
	for (i = 0; i<PACKAGE_SIZE; ++i){
		__writeByteOut(package[i]);
	}
	if (change_rs485_mode){
		setCommunicationToReceive();
	}
}

/*
 * 	##### Sending data to UART -> RS485 #####
 */
/* --- functions names that start with "__" are for use in this file only. --- */
void __writeByteOut(uint8_t out){
	/*	Wait for UART5 not to be busy (full).
	 *		Read UART-FR bit 5 -> 0 or 1 -> compare to != 0.
	 *		0 is not full, 1 is full.
	 *	Print data (8 bits) to UART5 DATA.
	 */
	while((UART5->FR & (1<<5)) != 0);
	UART5->DR = out;
}
