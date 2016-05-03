/*
 * Measurement.h
 *
 *  Created on: 10.04.2016
 *      Author: Max
 */

#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

void initAdc0(void);
void inintSyncCableInput(void);
void measurement_signal_generation(void);
void measurement_measure(unsigned short *);
void measurement_work(unsigned short *);

#endif /* MEASUREMENT_H_ */
