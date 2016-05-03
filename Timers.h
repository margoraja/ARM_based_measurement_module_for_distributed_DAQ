/*
 * Timers.h
 *
 *  Created on: 05.04.2016
 *      Author: Max
 */

#ifndef TIMERS_H_
#define TIMERS_H_

void initialize_timer0(int);
void delay_timer0(void);
void initialize_timer1(int);
void delay_timer1(void);
void initialize_timer2(int);
void delay_timer2(void);
void initialize_timer3(int);
void delay_timer3(void);
void initialize_timer4_for_measurement_delays(int);
void delay_timer4_for_measurement(void);
void initialize_timer5_for_UART_switching(int);
void delay_timer5_for_UART_switching(void);

void delay_counting(unsigned long);
#define rcgc (*((volatile unsigned long *)0x400FE108))

#endif /* TIMERS_H_ */
