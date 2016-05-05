
#include "ARM_measurement_unit.h"

void initialize_timer0(int delay){
	//timer
	SYSCTL->RCGCTIMER |= (1<<0);
	//timer off
	TIMER0->CTL &= ~(1<<0);
	//GPTM register
	TIMER0->CFG = 0x00000000;
	//TnMR
	TIMER0->TAMR |= (0x2<<0);
	//set count direction
	TIMER0->TAMR &= ~(1<<4);
	//count to
	TIMER0->TAILR = delay;
	//interupts not used
	TIMER0->CTL |= (1<<0);
}

void initialize_timer1(int delay){
	//timer
	SYSCTL->RCGCTIMER |= (1<<1);
	//timer off
	TIMER1->CTL &= ~(1<<0);
	//GPTM register
	TIMER1->CFG = 0x00000000;
	//TnMR
	TIMER1->TAMR |= (0x2<<0);
	//set count direction
	TIMER1->TAMR &= ~(1<<4);
	//count to
	TIMER1->TAILR = delay;
	//interupts not used
	TIMER1->CTL |= (1<<0);
}

void initialize_timer2(int delay){
	//timer
	SYSCTL->RCGCTIMER |= (1<<2);
	//timer off
	TIMER2->CTL &= ~(1<<0);
	//GPTM register
	TIMER2->CFG = 0x00000000;
	//TnMR
	TIMER2->TAMR |= (0x2<<0);
	//set count direction
	TIMER2->TAMR &= ~(1<<4);
	//count to
	TIMER2->TAILR = delay;
	//interupts not used
	TIMER2->CTL |= (1<<0);
}

void initialize_timer3(void){
	//timer
	SYSCTL->RCGCTIMER |= (1<<3);
	//timer off
	TIMER3->CTL &= ~(1<<0);
	//GPTM register
	TIMER3->CFG = 0x00000000;
	//TnMR
	TIMER3->TAMR |= (0x2<<0);
	//set count direction
	TIMER3->TAMR &= ~(1<<4);
	//count to
	TIMER3->TAILR = 16000000;
	//interupts not used
	TIMER3->CTL |= (1<<0);
}

void initialize_timer4_for_measurement_delays(int delay){
	//Enable timer.
	SYSCTL->RCGCTIMER |= (1<<4);
	//GPTM register
	TIMER4->CTL &= ~(1<<0);
	//GPTM register
	TIMER4->CFG = 0x00000000;
	//Using period mode.
	TIMER4->TAMR |= (0x2<<0);
	//Set count direction from up to down.
	TIMER4->TAMR |= (0<<4);
	//Counter from which to count to / from which to count from.
	TIMER4->TAILR = delay;
	//Activate timer.
	TIMER4->CTL |= (1<<0);
}

void initialize_delay_timer(void){
	//Enable timer.
	SYSCTL->RCGCTIMER |= (1<<5);
	//Turn timer off for configuration.
	TIMER5->CTL &= ~(1<<0);
	//GPTM register
	TIMER5->CFG = 0x00000000;
	//Using period mode.
	TIMER5->TAMR |= (0x2<<0);
	//Set count direction from up to down.
	TIMER5->TAMR |= (0<<4);
	//Counter from which to count to / from which to count from.
	TIMER5->TAILR = 10;
	//Activate timer.
	TIMER5->CTL |= (1<<0);
}

void delay_timer0(void){
	TIMER0->ICR |= (1<<0);
	while(1){
		//check if counted down
		if ((TIMER0->RIS & 0x00000001) == 1) {
			break;
		}
	}
	TIMER0->CTL &= ~(1<<0);
}

void delay_timer1(void){
	TIMER1->ICR |= (1<<0);
	while(1){
		//check if counted down
		if ((TIMER1->RIS & 0x00000001) == 1) {
			break;
		}
	}
}

void delay_timer2(void){
	TIMER2->ICR |= (1<<0);
	while(1){
		//check if counted down
		if ((TIMER2->RIS & 0x00000001) == 1) {
			break;
		}
	}
}

void delay_timer3(void){
	TIMER3->ICR |= (1<<0);
	while(1){
		//check if counted down
		if ((TIMER3->RIS & 0x00000001) == 1) {
			break;
		}
	}
}

void delay_timer4_for_measurement(void){
	TIMER4->CTL |= (1<<0);
	TIMER4->ICR |= (1<<0);
	while(1){
		//check if counted down
		if ((TIMER4->RIS & 0x00000001) == 1) {
			break;
		}
	}
	TIMER4->CTL |= (0<<0);
}

void delay_timer(int counts){
	int i;
	for (i = 0; i<counts; i++){
		TIMER5->ICR |= (1<<0);
		while(1){
			//check if counted down
			if ((TIMER5->RIS & 0x00000001) == 1) {
				break;
			}
		}
	}
}

void delay_counting(unsigned long halfsecs){
  unsigned long count;

  while(halfsecs > 0 ) { // repeat while still halfsecs to delay
    count = 15384600;
// originally count was 400000, which took 0.13 sec to complete
// later we change it to 400000*0.5/0.13=1538460 that it takes 0.5 sec
    while (count > 0) {
      count--;
    } // This while loop takes approximately 3 cycles
    halfsecs--;
  }
}
