#ifndef TIMERS_H_
#define TIMERS_H_

void initialize_timer0(int);
void delay_timer0(void);
void initialize_timer1(int);
void delay_timer1(void);
void initialize_timer2(int);
void delay_timer2(void);
void initialize_communication_timeout_timer(void);
void enabale_communication_timeout_timer(void);
void disabale_communication_timeout_timer(void);
void reset_communication_timeout_timer(void);
int get_communication_timeout_flag(void);
void initialize_measurement_delay_timer(int);
void measurement_delay(void);
void initialize_delay_timer(void);
void delay_timer(int);

void delay_counting(unsigned long);
#define rcgc (*((volatile unsigned long *)0x400FE108))

#endif /* TIMERS_H_ */
