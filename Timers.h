#ifndef TIMERS_H_
#define TIMERS_H_

void delay_timer_init(void);
void delay_timer(uint16_t);
void initialize_communication_timeout_timer(void);
void enabale_communication_timeout_timer(void);
void disabale_communication_timeout_timer(void);
void reset_communication_timeout_timer(void);
int get_communication_timeout_flag(void);
void initialize_measurement_delay_timer(int);
void measurement_delay(void);

//#define rcgc (*((volatile unsigned long *)0x400FE108))

#endif /* TIMERS_H_ */
