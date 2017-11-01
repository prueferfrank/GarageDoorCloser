/*
 * BlinkLed.cpp
 *
 *  Created on: 19.08.2017
 *      Author: Frank
 */

#include "BlinkLed.h"
#include "Arduino.h"

BlinkLed::BlinkLed() {
}

BlinkLed::~BlinkLed() {
}

void BlinkLed::begin(int pin, const char *s) {
  ledStateChange = 0;
  ledPin = pin;
  name = s;
  pinMode(ledPin, OUTPUT);
}
void BlinkLed::check() {
  if (ledOnTime == 0 || ledOffTime == 0) {
    return;
  }
  unsigned long currTime = millis();
  if (currTime >= ledStateChange) {
    if (ledState == HIGH) {
      ledStateChange = currTime + ledOffTime;
      ledState = LOW;
    } else {
      ledStateChange = currTime + ledOnTime;
      ledState = HIGH;
    }
    digitalWrite(ledPin, ledState);
  }
}

void BlinkLed::setBlinkRatio(int on, int off) {
  Serial.print("set blink ratio ");
  Serial.println(name);

  if (on == 0) {
    digitalWrite(ledPin, LOW);
    return;
  }
  if (off == 0) {
    digitalWrite(ledPin, HIGH);
    return;
  }

  digitalWrite(ledPin, HIGH);
  ledState = HIGH;
  ledStateChange = millis() + on;
  ledOnTime = on;
  ledOffTime = off;
}
