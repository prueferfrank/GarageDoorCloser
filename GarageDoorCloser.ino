/*
 Garagentorsteuerung

 Dieser Sketch sorgt dafür dass das Garagentor
 zwischen 22:00 und 6:00 dann automatisch geschlossen
 wird, wenn es länger als eine bestimmte Dauer offen steht
 oder schon vor 22:00 geöffnet wurde.

 Es kann aber auch manuell geschlossen werden.

 RTC SCL an A5
 RTC SDA an A4

 LED Pin 13 (controlLed)
 blinkt 200ms an 3000ms aus wenn das Tor oben ist (ausserhalb der Überwachungszeit)
 blinkt 200ms an 500ms aus wenn das Tor oben ist (innerhalb der Überwachunszeit)
 an wenn das Tor hoch oder runter fährt
 aus wenn das Tor unten ist
 */
#include "Arduino.h"
#include <RTClib.h>
#include <Time.h>
#include "BlinkLed.h"
RTC_DS1307 RTC;
BlinkLed controlLed;
BlinkLed doorOpenLed;
BlinkLed doorCloseLed;

/**
 * debugging = 0: normaler Betrieb über RTC-Zeit-Modul
 * debugging = 1: Testbetrieb
 */
#define xdebugging

// HIGH signal bedeutet Kontakt geschlosssen
#define doorCloseSensePin  11
#define doorCloseLedPin  9
// HIGH signal bedeutet Kontakt geschlosssen
#define doorOpenSensePin   12
#define doorOpenLedPin  10
#define doorControlPin  2

// Relais-Pin
#define controlLedPin  13

int doorOpenState = -1;
int doorCloseState = -1;
unsigned long int closeTime;
bool doorObserve = false;

// in seconds
#define observeTime 20

int writeCounter = 0;

int currentObservableTime = -1; //0: nein, 1: ja; -1: undefined

void setup() {
  Serial.begin(9600);
  controlLed.begin(controlLedPin, "ControlLed");
  controlLed.setBlinkRatio(0, 0);

  doorOpenLed.begin(doorOpenLedPin, "OpenLed");
  doorOpenLed.setBlinkRatio(0, 0);

  doorCloseLed.begin(doorCloseLedPin, "CloseLed");
  doorCloseLed.setBlinkRatio(0, 0);
}

void loop() {
#ifdef debugging
  DateTime now = RTC.now();
  // wenn sich die observable time aendert
  int newObservableTime = 0;
  if (isObservableTime(RTC.now())) {
    newObservableTime = 1;
  }
#else
  DateTime now = -1;
  int newObservableTime = 1;
  Serial.write("debugging mode\n");
#endif

  if (newObservableTime != currentObservableTime) {
    // wechsel von observabl nach not observable oder umgekehrt
    doorOpenState = -1;
    doorCloseState = -1;
    currentObservableTime = newObservableTime;
    if (currentObservableTime == 1) {
      writeTime(RTC.now());
      Serial.write("switch to observable time\n");
      controlLed.setBlinkRatio(10000, 0);
    } else {
      writeTime(RTC.now());
      Serial.write("switch to NON-observable time\n");
      controlLed.setBlinkRatio(0, 10000);
    }
  }

  if (currentObservableTime == 1) {
    doorOpen();
    doorClosed();
    if (doorObserve) {
      Serial.write('.');
      writeCounter++;
      if (writeCounter > 50) {
        writeCounter = 0;
        Serial.write("\n");
        Serial.write("closing in ");
        unsigned long int remTime = closeTime - now.unixtime();
        char buf[9];
        sprintf(buf, "%04lu", remTime);
        Serial.println(buf);
        Serial.write("\n");
      }
      if (now.unixtime() >= closeTime) {
        closeDoor();
      }
    }
  }

  // update door sense LED
  digitalWrite(doorCloseLedPin, digitalRead(doorCloseSensePin));
  digitalWrite(doorOpenLedPin, digitalRead(doorOpenSensePin));

  controlLed.check();
  doorOpenLed.check();
  doorCloseLed.check();
  delay(50);
}

boolean doorClosed() {
  int newState = digitalRead(doorCloseSensePin);
  digitalWrite(doorCloseLedPin, newState);
  if (newState != doorCloseState) {
    writeTime(RTC.now());
    if (newState == LOW) {
      Serial.println("door down");
    } else {
      Serial.println("door moving up");
    }
    doorCloseState = newState;
  }
  return (newState == LOW);
}

boolean doorOpen() {
  int newState = digitalRead(doorOpenSensePin);
  digitalWrite(doorOpenLedPin, newState);
  if (newState != doorOpenState) {
    writeTime(RTC.now());
    if (newState == LOW) {
      Serial.println("door top");
      if (isObservableTime(RTC.now())) {
        controlLed.setBlinkRatio(800, 800);

        startObserve();
      }
    } else {
      Serial.println("door moving down");
      if (isObservableTime(RTC.now())) {
        stopObserve();
      }
    }
    doorOpenState = newState;
  }
  return (newState == LOW);
}

void startObserve() {
  // Ueberwachung startet
  Serial.println("start observing");
  doorObserve = true;
  closeTime = RTC.now().unixtime() + observeTime;
  DateTime dt7(closeTime);
  writeTime(dt7);

}

void stopObserve() {
  Serial.println("stop observing");
  doorObserve = false;
  controlLed.setBlinkRatio(10000, 0);
}

void closeDoor() {
  doorObserve = false;
  Serial.println("close door now");
  digitalWrite(doorControlPin, LOW);
  delay(1000);
  controlLed.setBlinkRatio(100, 100);

  while (doorOpen()) {
    delay(1000);
    Serial.println(" door not moving");
  }
  digitalWrite(doorControlPin, HIGH);
  Serial.println(" door moving ");
}

bool isObservableTime(DateTime dt) {
#ifdef debugging
  return (dt.hour() < 7 || dt.hour() >= 22);
#else
  return (dt.second() >= 30);
#endif
}

void writeTime(DateTime currentDate) {
  Serial.print(currentDate.hour());
  Serial.print(':');
  Serial.print(currentDate.minute());
  Serial.print(':');
  Serial.print(currentDate.second());
  Serial.print(' ');
  Serial.print(currentDate.day());
  Serial.print('.');
  Serial.print(currentDate.month());
  Serial.print('.');
  Serial.println(currentDate.year());
}

