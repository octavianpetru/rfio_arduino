#include "rfio_arduino.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>
#include <Print.h>
#include <stdio.h>
#include <stdlib.h>

#include "RCSwitch.h"

unsigned long before;
unsigned int indexReceive = 0;

RCSwitch mySwitch = RCSwitch();

void setup() {
	Serial.begin(9600);

	// TX_POWER_PIN
	pinMode(TX_POWER_PIN, OUTPUT);
	digitalWrite(TX_POWER_PIN, LOW);
	// enable transmit
	mySwitch.enableTransmit(TX_PIN);
	mySwitch.setProtocol(1);
	mySwitch.setRepeatTransmit(8);

	// RX_POWER_PIN
	pinMode(RX_POWER_PIN, OUTPUT);
	digitalWrite(RX_POWER_PIN, HIGH);
	// enable receive
	mySwitch.enableReceive(digitalPinToInterrupt(RX_PIN)); // Receiver on interrupt 0 => that is pin #2

	Serial.print("RCSWITCH_MAX_CHANGES: ");
	Serial.println(RCSWITCH_MAX_CHANGES);
	Serial.print("\n");

	before = millis();
	indexReceive = 0;
}

void loop() {

	if (mySwitch.available()) {
		unsigned long receivedValue = mySwitch.getReceivedValue();
		Serial.println(receivedValue, BIN);

		// debug receiving times
//		unsigned int* receivedRawdata = mySwitch.getReceivedRawdata();
//		for (int i = 0; i < RCSWITCH_MAX_CHANGES; i++) {
//			Serial.println(*(receivedRawdata + i));
//		}
//		Serial.print('\n');

		mySwitch.resetAvailable();

		TemperatureMessage *temperatureMessage = readReceivedValue(
				receivedValue);

		char outputString[160];
		sprintf(outputString,
				"indexReceive %3d / startLabel: %s / id: %3d / canal: %1d / temperature: %4d / temperatureStr: %s / endLabel: %s / time: %lu\n",
				(*temperatureMessage).indexReceive,
				(*temperatureMessage).startLabel, (*temperatureMessage).id,
				(*temperatureMessage).canal, (*temperatureMessage).temperature,
				(*temperatureMessage).temperatureStr,
				(*temperatureMessage).endLabel, (*temperatureMessage).time);
		Serial.println(outputString);

		delay(2000);

		char s[13] = { '\0' };
		toBinStr((*temperatureMessage).temperature, s, 12);
		char str[38] = { '\0' };
		sprintf(str, "1001000110010001%s110011000", s);
		digitalWrite(TX_POWER_PIN, HIGH);
		mySwitch.sendString(str);
		digitalWrite(TX_POWER_PIN, LOW);

		char outputStringSend[160];
		sprintf(outputStringSend,"%3d sent %3d\n", (*temperatureMessage).indexReceive, (*temperatureMessage).temperature);
		Serial.println(outputStringSend);

		free(temperatureMessage);
	}
}

void toBinStr(long value, char* output, int i) {
	output[i] = '\0';
	for (i = i - 1; i >= 0; --i, value >>= 1) {
		output[i] = (value & 1) + '0';
	}
}

/*
 * 10010111010100000000111010111100
 * missing the last 5 bits (37 in total)
 */
TemperatureMessage *readReceivedValue(unsigned long receivedValue) {

	// allocate struct
	TemperatureMessage *temperatureMessage;
	temperatureMessage = (TemperatureMessage*) malloc(
			sizeof(TemperatureMessage));

	// start label
	short startLabelLong = (receivedValue >> 28) & 15;
	toBinStr(startLabelLong, (*temperatureMessage).startLabel, 4);

	// id
	(*temperatureMessage).id = (receivedValue >> 20) & 255;

	// canal
	(*temperatureMessage).canal = (receivedValue >> 16) & 15;
	(*temperatureMessage).canal += 1;

	// temperature
	(*temperatureMessage).temperature = (receivedValue >> 4) & 4095;

	// 2^12 = 4096
	if ((*temperatureMessage).temperature > 2047) {
		(*temperatureMessage).temperature = -(4096
				- (*temperatureMessage).temperature);
	}

	// temperature as string
	short upperValue = (*temperatureMessage).temperature / 10 * 10;
	short lowerValue = abs((*temperatureMessage).temperature - upperValue);
	upperValue /= 10;

	sprintf((*temperatureMessage).temperatureStr, "%3d%s%1d", int(upperValue),
			".", lowerValue);
	(*temperatureMessage).temperatureStr[5] = '\0';

	// end label
	short endLabelLong = receivedValue & 15;
	toBinStr(endLabelLong, (*temperatureMessage).endLabel, 4);

	// index of receive
	(*temperatureMessage).indexReceive = ++indexReceive;

	// time since last
	(*temperatureMessage).time = millis() - before;
	before = millis();

	return temperatureMessage;
}
