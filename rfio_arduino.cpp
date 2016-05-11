#if not defined(ARDUINO) && ARDUINO >= 100
#include "rfio_arduino.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>
#include <Print.h>
#include <stdio.h>
#endif

#include "RCSwitch.h"

#define POT_PIN A0
#define LED_PIN 3
#define RX_PIN 2
#define TX_PIN 6
#define POWER_PIN 4

short potValue = 0;
short ledValue = 0;
int count = 0;
int oldValue = 0;
int countSameValue = 0;
unsigned long before;
char disabled = 0;
unsigned int indexReceive = 0;

RCSwitch mySwitch = RCSwitch();

void tobinstr(long value, char* output, int i) {
	output[i] = '\0';
	for (i = i - 1; i >= 0; --i, value >>= 1) {
		output[i] = (value & 1) + '0';
	}
}

void setup() {
	Serial.begin(9600);

	pinMode(LED_PIN, OUTPUT);
	pinMode(POWER_PIN, OUTPUT);

	digitalWrite(POWER_PIN, HIGH);
	digitalWrite(LED_PIN, HIGH);
	mySwitch.enableReceive(digitalPinToInterrupt(RX_PIN)); // Receiver on interrupt 0 => that is pin #2
	mySwitch.setProtocol(6);
	disabled = 0;

	Serial.println(RCSWITCH_MAX_CHANGES);

	before = millis();
	indexReceive = 0;
}

void loop() {
	/*
	 potValue = analogRead(POT_PIN);
	 ledValue = map(potValue, 0, 1023, 0, 255);

	 analogWrite(LED_PIN, ledValue);
	 Serial.print("ledValue: ");
	 Serial.println(ledValue);
	 delay(50);
	 */

	if (disabled == 1 & millis() - before > 54000) {
		digitalWrite(POWER_PIN, HIGH);
		digitalWrite(LED_PIN, HIGH);
		mySwitch.enableReceive(digitalPinToInterrupt(RX_PIN));
		disabled = 0;
	}

	if (mySwitch.available()) {
		/*
		 if (disabled == 0) {
		 digitalWrite(POWER_PIN, LOW);
		 digitalWrite(LED_PIN, LOW);
		 mySwitch.disableReceive();
		 disabled = 1;
		 }
		 */
		unsigned long receivedValue = mySwitch.getReceivedValue();
		Serial.println(receivedValue, BIN);

		unsigned int* receivedRawdata = mySwitch.getReceivedRawdata();
		mySwitch.resetAvailable();

		//    for (int i = 0; i < RCSWITCH_MAX_CHANGES; i++) {
		//      Serial.println(*(receivedRawdata + i));
		//    }
		//    Serial.print('\n');

		short startlabl = (receivedValue >> 28) & 15;
		char startlab[5] = { '\0' };
		tobinstr(startlabl, startlab, 4);

		short id = (receivedValue >> 20) & 255;

		short canal = (receivedValue >> 16) & 15;

		short temp = (receivedValue >> 4) & 4095;

		// 2^12 = 4096
		if (temp > 2047) {
			temp = -(4096 - temp);
		}

		short upperv = temp / 10 * 10;
		short lowerv = abs(temp - upperv);
		upperv /= 10;

		char strtemp[6] = { ' ' };
		sprintf(strtemp, "%3d%s%1d", int(upperv), ".", lowerv);
		strtemp[5] = '\0';

		short endlabell = receivedValue & 15;
		char endlabel[5] = { '\0' };
		tobinstr(endlabell, endlabel, 4);

		indexReceive++;

		char str[128];
		sprintf(str,
				"num %3d / startlab: %s / id: %3d / canal: %1d / temp: %4d / temp: %s / endlabel: %s / time: %lu\n",
				indexReceive, startlab, id, (canal + 1), temp, strtemp,
				endlabel, millis() - before);
		Serial.println(str);

		before = millis();

	}

}
