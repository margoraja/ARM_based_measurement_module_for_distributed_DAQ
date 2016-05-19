#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

void initializeUART5(void);
void readPackage(uint8_t *);
void sendResults(uint8_t *);
void sendState(void);

#endif /* COMMUNICATION_H_ */
