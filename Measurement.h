#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

void initAdc0(void);
void inintSyncCableInput(void);
void measurement_signal_generation(void);
void measurement_measure(uint8_t *);
void measurement_work(uint8_t *);

#endif /* MEASUREMENT_H_ */
