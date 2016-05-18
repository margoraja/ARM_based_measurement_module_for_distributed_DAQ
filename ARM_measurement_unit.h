#ifndef ARM_MEASUREMENT_UNIT_H_
#define ARM_MEASUREMENT_UNIT_H_
/* ===========================================================================================
 * ==========	Include LM4F230H5QR.h header which corresponds to TM4C123GH6PM MCU	==========
 * ==========			That is used on EK-TM4C123GXL laynchpad board. 				==========
 * ===========================================================================================
 */
#include <LM4F230H5QR.h>
#include "Timers.h"
#include "Utils.h"
#include <string.h>

/* =======================================================================
 * ==========	LED usage and feedback during device operation	==========
 * =======================================================================
 *
 *  ***	BLUE LED:
 * 				1) When generatin signal
 * 				2) Performing measurements
 *
 * 	*** RED LED:
 * 				1) Measurement results are stored on device but not sent to controller.
 *
 * 	*** GREEN LED:
 * 				1) Sending measurement results out
 *
 * 	*** LED OFF:
 * 				1) Any other state
 *
 */

/*	======================================================================================================
 * 	==========	Predefined immutable variables and macros for measurement unit configuration.	==========
 * 	======================================================================================================
 */
/* ===========================
 * ==========	ID	==========
 * ===========================
 * IDs ised by measurement unit and global ID that is used for communication with all devices in network.
 * 	Used by controlelr only.*/
#define ID 97 //ID
#define GLOBAL_ID 98//0

/* ===================================================
 * ==========	Communication configuration	==========
 * ===================================================
 * 	Required baud-rate for communication is 115200.
 * 	Integer BRD = 16000000/(16*115200) = 8,... -> 8
 * 	Fraction BRD = (BRD - integer(BRD))*64 + 0,5 = 44,... -> 44
 * 	115200: iBRD 8, fBRD 44*/
#define PACKAGE_SIZE 4
#define CRC_INCLUDED 1
#define CRC_BYTE_COUNT 1 //Currently not supported
#define CRC_BIT_CHECK_FOR_FLIP 0b10000000
#define CRC_BIT_FLIP 0b00000111
#define BAUD_RATE 115200
#define INTEGER_BRD 8
#define FLOAT_BRD 44
#define RS485_RECEIVE 0 //Receive mode level. High = 1, Low = 0.
#define UART_SWITCHING_DELAY 10 // Uart switching delay in milliseconds
#define COMMUNICATION_TIMEOUT 16000000 //For discarding incomplete packages.

/* ===================================================================
 * ==========	Predefined commands sent by controller.		==========
 * ===================================================================
 */
#define NULL_BYTE 0
#define START_WORK 97 //1
#define START_SIGNAL 98 //2
#define STOP_SIGNAL 99 //3
#define START_MEASURING 100 //4
#define SEND_RESULTS 101 //5
#define DELETE_RESULTS 102 //6
#define USE_SYNC_CABLE 103 //7
#define GET_STATE 104 //255
// Maybe it's necessary to have "null package" for reseting communication for all devices?
// There are issues with communication when other products in network are switching between receive and transmitting.


/* =======================================================
 * ==========	Measurement related parameters	==========
 * =======================================================
 * 	How many times measurements are done.
 * 	Limited to allocatable memory size, limit is: .
 *	1 = 62.5 nano seconds (+ Internal delay which is x ns) if core clock is 16MHz.*/
#define SAMPLE_COUNT 1000 /* Limit is between 16-17k currently */
#define SAMPLE_DELAY 1000 /* Must find out how fast it actually is capable of measuring. */
#define OVERWRITE_OLD_RESULTS 0 /* 0 to keep old results if not sent, otherwise will overwrite, setting it to 1 may cause iterrupts to perform two measurements */

/* ===========================================================
 * ==========	Signal generator, PWM, prameters	==========
 * ===========================================================
 * Load: Period, calculated by: -> (System clock in kHz/ divider)/load = period in ms -> (System clock / divider)/period in ms = load
 * Load -> length of signal being generated.
 * Width -> how many counts out of LOAD is LOW -> 0 to PWM_LOAD.*/
#define PWM_LOAD 10000
#define PWM_WIDTH 3333
// Uses system clock divider or not. 0 sets system clock as source clock for PWM.
#define USE_CLOCK_DIVIDER 1
// See page 255 on data sheet.
#define CLOCK_DIVIDER 7

/* ===============================================================================
 * ========== Data complete(whole), start, continue and end bit masks.	==========
 * ===============================================================================
 */
#define DATA_WHOLE (0<<7)|(0<<6)
#define DATA_START (0<<7)|(1<<6)
#define DATA_CONTINUE (1<<7)|(0<<6)
#define DATA_END (1<<7)|(1<<6)

/* ===============================
 * ==========	DEBUG	==========
 * ===============================
 * Prints debug information to UART instead of usual. E.g. values are represented binary in human readable form, 1s and 0s.
 *
 * WARNING!
 * Device will send more results out in various stats when it's not normally expected!
 * Send one package (4 bytes) at the time to device.*/
#define PRINT_DEBUG 0

/* ===================================
 * ==========	Bit masks	==========
 * ===================================
 */
#define INVALID_PACKAGE_BIT 6
#define READY_TO_WORK_BIT 5
#define MEASUREMENTS_SENT_BIT 4
#define MEASUREMENTS_PRESENT_BIT 3

#define GET_READY_TO_WORK_BIT (1 & (state>>READY_TO_WORK_BIT))
#define GET_MEASUREMENTS_SENT_BIT (1 & (state>>MEASUREMENTS_SENT_BIT))
#define GET_MEASUREMENTS_PRESENT_BIT (1 & (state>>MEASUREMENTS_PRESENT_BIT))
#define GET_INVALID_PACKAGE_BIT (1 & (state>>INVALID_PACKAGE_BIT))

/*================================================
* ==========		Interrupt related	==========
* ================================================
 */
#define CHECK_INT_DURING_COMMUNICATION ((byte_counter) > 0 ? 1 : 0)


/* ===================================================================================
 * ==========	Predfined mutable variables for measurement unit operation.	==========
 * ===================================================================================
 */
/* ===================================
 * ==========	Variables	==========
 * ===================================
 */
uint8_t state;
extern uint8_t measurement_results[SAMPLE_COUNT]; //Should be dynamically allocated.
uint8_t interrupt_occurred;//Used to keep track if interrupt was received during the pacakge reading. Will be left 1 if byte_counter is above 0.
uint8_t byte_counter;
uint8_t communication_timeout;

#endif /* ARM_MEASUREMENT_UNIT_H_ */
