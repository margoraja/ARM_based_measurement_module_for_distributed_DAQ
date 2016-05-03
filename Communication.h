/*
 * Communication.h
 *
 *  Created on: 05.04.2016
 *      Author: Max
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

void initializeUART5(void);
void readPackage(unsigned char *);
void writeCharPackageOut(unsigned char *, short);
void writeIntPackageOut(int *, short);
void sendResults(unsigned short *);
void sendState(void);

#endif /* COMMUNICATION_H_ */
