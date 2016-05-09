
#include "ARM_measurement_unit.h"
#include "CRC.h"

void setCommunicationToTransmit(void);
void setCommunicationToReceive(void);
void initializeRS485Controller(void);
uint8_t readFirstByte(void);
uint8_t readNextBytes(void);
void __writeCharByteOut(char);
void __writeIntByteOut(int);
void debug_writeStringOut(char *, uint8_t);
void debug_writeByteInBinary(char, uint8_t);
void debug_writePackageBytesInBinary(uint8_t [], uint8_t);
void writeCharPackageOut(uint8_t *, uint8_t);

//extern uint8_t measurement_results[SAMPLE_COUNT];

/*
void configureUART5(void);
void readPackage(int *);
void writePackageOut(int *);
void sendResults(uint8_t);
*/

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
	setGreenLED();
	delay_timer(UART_SWITCHING_DELAY*2);
	UART5->CTL = (0<<0);
	GPIOB->DATA = ~(RS485_RECEIVE<<4);
	UART5->CTL = (1<<0)|(1<<8);
}

void setCommunicationToReceive(void){
	/* First of all wait just a little to ensure that everything is sent before disabling UART.
	 * 1. Disable UART5
	 * 2. Reconfigure RS485 to receive
	 * 3. Enable UARt5 with receive
	 */
	delay_timer(UART_SWITCHING_DELAY/2);
	UART5->CTL = (0<<0);
	GPIOB->DATA = (RS485_RECEIVE<<4);
	UART5->CTL = (1<<0)|(1<<9);
	setLED();
}

void initializeRS485Controller(void){
	//Enable clock to PBx GPIO block.
	SYSCTL->RCGCGPIO |= (1<<1);
	//Enable PB4 port control for controlling RS485 receive and transmit.
	GPIOB->PCTL |= (1<<16);
	//Disable analogue mode.
	GPIOB->AMSEL |= ~(1<<4);
	//Enable PB4 as direction output
	GPIOB->DIR |= (1<<4);
	//Disable Alternate functions
	GPIOB->AFSEL |= ~(1<<4);
	//Enable digital
	GPIOB->DEN |= (1<<4);
}

void initializeUART5(void){
	// Initialize timer5 for changing RS485 mode.

	//Initlize RS485 chip controller. Sets to listening.
	initializeRS485Controller();

	/*		UART and PIN configuration
	 *Step 1. 	Enables UART5 module, PE4-PE5. Page 344
	 *			0 disabled, 1 enabled
	 */
	SYSCTL->RCGCUART |= (1<<5);

	/*
	 * Step 2. 	Enables clock to appropiate GPIO. Required to set it to E, which is 0. Page 340.
	 *			There for enable clock to GPIOE.
	 *			0 disabled, 1 enabled
	 */
	SYSCTL->RCGCGPIO |= (1<<4);
	//Step 3. 	Enable required pins to GPIO AFSEL. Must set pins for GPIOE, required are PE4 and PE5 (4 and 5).
	//			0 controlled by register, 1 controller by alternate hardware function.
	GPIOE->AFSEL |= (1<<5)|(1<<4);

	/* Step 4.	Current level and/or slew rate are not required.*/

	/* Step 5. 	Assigning UART signals to specific pins. PMCx Bit Field Encoding for PE4-PE5 is 1.
	 * 			PMC0-7 represent x in PAx. Eg 4 and 5 for PE4 and PE5.
	 *			PMC0 starts with 0 and PMC1 stastrs with 4. There for 1 must be set into 16 (PMC4)and 20 (PMC5) locations.
	 *			0 disabled, 1 enabled
	 */
	GPIOE->PCTL |= (1<<16)|(1<<20);

	//			Since UART5 is digital signal, enable DigitalENable signals for specific PINs.
	//			0 disabled, 1 enabled
	GPIOE->DEN |= (1<<4)|(1<<5);

	//			Disabe analogue functions
	GPIOE->AMSEL |= ~(1<<4)|(1<<5);

	/*		Communikation clocks, baud-rate.
	 *		Using default clock which is 16MHz.
	 *		Required baud-rate for communication is 115200.
	 *		Integer BRD = 16000000/(16*115200) = 8,...-> 8
	 *		Fraction BRD = (BRD - integer(BRD))*64 + 0,5 = 44,... -> 44
	 * 		115200: iBRD 8, fBRD 44
	 */

	//Step 1.	Disable UART by clearing the UARTEN bit in the UARTCTL register
	UART5->CTL &= ~(1<<0);

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
		byte_counter = 0;
		/* Reset interrupt occurred value. */
		setIntOccurredValue(0);
		/* Reset communication timeout value. */
		communication_timeout = 0;
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
 * 	##### Sending data to UART -> RS485 #####
 */
/* --- functions names that start with "__" are for use in this file only. --- */
void __writeCharByteOut(char c){
	/*	Wait for UART5 not to be busy (full).
	 *		Read UART-FR bit 5 -> 0 or 1 -> compare to != 0.
	 *		0 is not full, 1 is full.
	 *	Print data (8 bits) to UART5 DATA.
	 */
	while((UART5->FR & (1<<5)) != 0);
	UART5->DR = c;
}

void __writeIntByteOut(int c){
	/*	Wait for UART5 not to be busy (full).
	 *		Read UART-FR bit 5 -> 0 or 1 -> compare to != 0.
	 *		0 is not full, 1 is full.
	 *	Print data (8 bits) to UART5 DATA.
	 */
	while((UART5->FR & (1<<5)) != 0);
	UART5->DR = c;
}

void writeIntPackageOut(uint8_t *package, uint8_t change_rs485_mode){
	/*
	 * Int is casted to char, so firs 8 lower bits are only sent out.
	 */
	int i;
	if (change_rs485_mode){
		setCommunicationToTransmit();
	}
	for (i = 0; i<PACKAGE_SIZE; ++i){
		__writeCharByteOut((char)package[i]);
	}
	debug_writeStringOut("\n\r", 0);
	if (change_rs485_mode){
		setCommunicationToReceive();
	}
}

void writeCharPackageOut(uint8_t *package, uint8_t change_rs485_mode){
	int i;
	if (change_rs485_mode){
		setCommunicationToTransmit();
	}
	for (i = 0; i<PACKAGE_SIZE; ++i){
		__writeCharByteOut((char)package[i]);
	}
	if (change_rs485_mode){
		setCommunicationToReceive();
	}
}

void sendResults(uint8_t results[]){
	if (GET_MEASUREMENTS_PRESENT_BIT){
		int counter = 0;
		uint8_t package[PACKAGE_SIZE];
		package[0] = 107;
		setCommunicationToTransmit();
		while (SAMPLE_COUNT >= counter){
			/*
			 * results[counter] & 0xFC0; //ADC result 0-6 bit mask: 0b000000111111 = 63 = 3F
			 * plus data start mask (0b01******)
			 */
			package[1] = DATA_START|(results[counter]>>6);
			/*
			 * ADC result 7-11 bit mask: 0b111111000000 = 4032 = FC0
			 * plus data end mask (0b11******)
			 */
			package[2] = DATA_END|results[counter] & 0x3F;
			/*
			 * CRC calculated from package[0] to package[2]
			 */
			package[3] = calculate_CRC(package);

			if (PRINT_DEBUG){
				debug_writeByteInBinary(results[counter]>>8, 0);
				debug_writeByteInBinary(results[counter] & 0xFF, 0);
				debug_writeStringOut(" --> ", 0);
				debug_writePackageBytesInBinary(package, 0);
				debug_writeStringOut("\n\r\n\r", 0);
			}else{
				writeCharPackageOut(package, 0);
			}
			counter++;
			delay_timer(10);
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
	writeCharPackageOut(package, 0);
	setCommunicationToReceive();
	clearInvalidPackageBit();
}

/*
 * Debug functions for sending data out.
 */

void debug_writeStringOut(char * string, uint8_t change_rs485_mode){
	if (change_rs485_mode){
		setCommunicationToTransmit();
	}
	while(*string){
		__writeCharByteOut(*(string++));
	}
	if (change_rs485_mode){
		setCommunicationToReceive();
	}
}

void debug_writeByteInBinary(char byte, uint8_t change_rs485_mode){
	int j;
	if (change_rs485_mode){
		setCommunicationToTransmit();
	}
	/* Every bit in byte */
	for (j=7; j>=0; --j){
		/* If it is 1, write 1 to output */
		if ((byte & (1<<j)) != 0){
			debug_writeStringOut("1", 0);
		}else{ /* Else write 0 to output */
			debug_writeStringOut("0", 0);
		}
	}
	if (change_rs485_mode){
		setCommunicationToReceive();
	}
}

void debug_writePackageBytesInBinary(uint8_t package[], uint8_t change_rs485_mode){
	/*
	 * For debug purposes it's sometimes better to see what is sent out.
	 */
	int i;
	if (change_rs485_mode){
		setCommunicationToTransmit();
	}
	/* Every package byte */
	for (i = 0; i<PACKAGE_SIZE; ++i){
		debug_writeByteInBinary(package[i], 0);
		if (i<3){
			debug_writeStringOut(" -- ", 0);
		}else{
			debug_writeStringOut("\n\r\n\r", 0);
		}
	}
	if (change_rs485_mode){
		setCommunicationToReceive();
	}
}
