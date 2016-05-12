#ifndef _rfio_arduino_H_
#define _rfio_arduino_H_

#define RX_PIN 2
#define RX_POWER_PIN 4
#define TX_PIN 3
#define TX_POWER_PIN 12

typedef struct {
	unsigned int indexReceive;
	char startLabel[5];
	short id;
	short canal;
	short temperature;
	char temperatureStr[6];
	char endLabel[5];
	unsigned long time;
} TemperatureMessage;

void toBinStr(long value, char* output, int i);
TemperatureMessage *readReceivedValue(unsigned long receivedValue);

#endif
