#if not defined(ARDUINO)
#include "rfio_arduino.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>
#include <Print.h>
#include <stdio.h>
#endif

#include "RCSwitch.h"

#define RX_PIN 2
#define RX_POWER_PIN 4
#define TX_PIN 3
#define TX_POWER_PIN 12

unsigned long before;
unsigned int indexReceive = 0;

RCSwitch mySwitch = RCSwitch();

void toBinStr(long value, char* output, int i) {
	output[i] = '\0';
	for (i = i - 1; i >= 0; --i, value >>= 1) {
		output[i] = (value & 1) + '0';
	}
}

void setup() {
	Serial.begin(9600);

	pinMode(TX_POWER_PIN, OUTPUT);
	digitalWrite(TX_POWER_PIN, LOW);
	mySwitch.disableTransmit();
	pinMode(TX_PIN, OUTPUT);
	digitalWrite(TX_PIN, LOW);

	pinMode(RX_POWER_PIN, OUTPUT);
	digitalWrite(RX_POWER_PIN, HIGH);
	mySwitch.enableReceive(digitalPinToInterrupt(RX_PIN)); // Receiver on interrupt 0 => that is pin #2
	mySwitch.setProtocol(6);

	Serial.print("RCSWITCH_MAX_CHANGES: ");
	Serial.println(RCSWITCH_MAX_CHANGES);

	before = millis();
	indexReceive = 0;
}

void loop() {

	if (mySwitch.available()) {
		unsigned long receivedValue = mySwitch.getReceivedValue();
		Serial.println(receivedValue, BIN);

//		unsigned int* receivedRawdata = mySwitch.getReceivedRawdata();
//		for (int i = 0; i < RCSWITCH_MAX_CHANGES; i++) {
//			Serial.println(*(receivedRawdata + i));
//		}
//		Serial.print('\n');

		mySwitch.resetAvailable();

		short startLabelLong = (receivedValue >> 28) & 15;
		char startLabel[5] = { '\0' };
		toBinStr(startLabelLong, startLabel, 4);

		short id = (receivedValue >> 20) & 255;

		short canal = (receivedValue >> 16) & 15;

		short temperature = (receivedValue >> 4) & 4095;

		// 2^12 = 4096
		if (temperature > 2047) {
			temperature = -(4096 - temperature);
		}

		short upperValue = temperature / 10 * 10;
		short lowerValue = abs(temperature - upperValue);
		upperValue /= 10;

		char temperatureStr[6] = { ' ' };
		sprintf(temperatureStr, "%3d%s%1d", int(upperValue), ".", lowerValue);
		temperatureStr[5] = '\0';

		short endLabelLong = receivedValue & 15;
		char endLabel[5] = { '\0' };
		toBinStr(endLabelLong, endLabel, 4);

		indexReceive++;

		char outputString[160];
		sprintf(outputString,
				"indexReceive %3d / startLabel: %s / id: %3d / canal: %1d / temperature: %4d / temperatureStr: %s / endLabel: %s / time: %lu\n",
				indexReceive, startLabel, id, (canal + 1), temperature,
				temperatureStr, endLabel, millis() - before);
		Serial.println(outputString);

		before = millis();
	}
}
