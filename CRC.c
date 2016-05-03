
#include "ARM_measurement_unit.h"

/*
 * Returns 0 if all OK
 * Returns 1 if CRC does not match with the payload.
 */
short checkCrc(unsigned char package[]){
	return 0;
}

unsigned char calculate_CRC(unsigned short id, unsigned char first_byte, unsigned char second_byte){
	/*
	 * Use Device ID (ID), first and second byte (first_byte, second_byte) to calculate CRC.
	 * Return crc value.
	 */
	return 35;
}
