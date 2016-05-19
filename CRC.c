
#include "ARM_measurement_unit.h"

uint8_t _calculate_8bit_crc(uint8_t, uint8_t);

uint8_t calculate_CRC(uint8_t package[]){
	/*
	 * Use Device ID (ID), first and second byte (first_byte, second_byte) to calculate CRC.
	 * Return crc value.
	 */
	uint8_t i;
	/* Init CRC value. */
	uint8_t crc = 0;

	/* Calculate CRC from payload. */
	for (i = 0; i<(PACKAGE_SIZE - CRC_BYTE_COUNT); i++){
		crc = _calculate_8bit_crc(crc, package[i]);
	}
	return crc;
}

uint8_t _calculate_8bit_crc (uint8_t inCrc, uint8_t inData){
   uint8_t i;
   uint8_t crc;
   /* Add data without clearing previously calculated crc (if was set), using XOR. */
   crc = inCrc ^ inData;

   /* 8 times (shifting to left by one and fliping three lowest bits) or shifting to left by one. */
   for ( i = 0; i < 8; i++ ){
	   if (( crc & CRC_BIT_CHECK_FOR_FLIP ) != 0 ){
		   crc <<= 1;
		   crc ^= CRC_BIT_FLIP;
	   } else {
		   crc <<= 1;
	   }
   }
   return crc;
}

/*
 * Check if CRC is OK or not.
 * Returning 0 in case of error, 1 otherwise.
 */
uint8_t checkCrcIsOk(uint8_t package[]){
	if (calculate_CRC(package) != package[PACKAGE_SIZE - 1]){
		return 0;
	} else {
		return 1;
	}
}
