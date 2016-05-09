/*
 * Communication.h
 *
 *  Created on: 05.04.2016
 *      Author: Max
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

void initializeUART5(void);
void readPackage(uint8_t *);
void writeCharPackageOut(uint8_t *, uint8_t);
void writeIntPackageOut(int *, uint8_t);
void sendResults(uint8_t *);
void sendState(void);

#endif /* COMMUNICATION_H_ */
