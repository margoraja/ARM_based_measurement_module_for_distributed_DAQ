#ifndef UTILS_H_
#define UTILS_H_

void initializeLEDs(void);
void setBlueLED(void);
void setGreenLED(void);
void setRedLED(void);
void setLEDoff(void);
void setLED(void);

void initIntOccurredValue(void);
void setIntOccurredValue(uint8_t);

void initState(void);

void setReadyToWorkBit(void);
void setMeasurementsResultsPresentBit(void);
void setMeasurementsSentBit(void);
void setInvalidPackageBit(void);

void clearReadyToWorkBit(void);
void clearMeasurementsResultsPresentBit(void);
void clearMeasurementsSentBit(void);
void clearInvalidPackageBit(void);

#endif /* UTILS_H_ */
