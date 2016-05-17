#ifndef UTILS_H_
#define UTILS_H_

void initializeLEDs(void);
void blueLedOn(void);
void greenLedOn(void);
void redLedOn(void);
void LedOff(void);
void setLed(void);

void initIntOccurredValue(void);
void setIntOccurredValue(uint8_t);

void initState(void);

void setReadyToWorkBit(void);
void setMeasurementsResultsPresentBit(void);
void setMeasurementsSentBit(void);
void setInvalidPackageBit(void);

uint8_t GetBit(volatile uint32_t, uint8_t);
void SetBit(volatile uint32_t*, uint8_t, uint8_t);

void clearReadyToWorkBit(void);
void clearMeasurementsResultsPresentBit(void);
void clearMeasurementsSentBit(void);
void clearInvalidPackageBit(void);

#endif /* UTILS_H_ */
