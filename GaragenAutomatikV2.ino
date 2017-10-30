/*
  Garagentorsteuerung

  Dieser Sketch sorgt dafür dass das Garagentor
  zwischen 22:00 und 6:00 dann automatisch geschlossen
  wird, wenn es länger als 10 Minuten offen stehen bleibt
  oder schon vor 22:00 geöffnet wurde.
   Es werden jeweils über die serielle Schnittstelle folgende
  Aktionen ausgegeben:
  TorOben: wenn die torOben lichtschranke von offen nach unterbrochen wechselt
  torUnten: wenn die torUnten
  Bewegung runter wenn die torOben von unterbrochen nach offen wechselt
  Bewegung hoch wenn der Kontakt TorUnten verlassen wird.

  Uhr (RTC)
  ---------
  Die Uhr ist ein fertiges Modul welches mit I2c angebunden wird. 
  Zum stellen der Uhr muss der Sketch neu kompiliert werden und hochgeladen werden. 
  
  RTC SCL an A5
  RTC SDA an A4
  
  

  LED Pin 13
  blinkt 200ms an 3000ms aus wenn das Tor oben ist (ausserhalb der Überwachungszeit)
  blinkt 200ms an 500ms aus wenn das Tor oben ist (innerhalb der Überwachunszeit)
  an wenn das Tor hoch oder runter fährt
  aus wenn das Tor unten ist
*/
#include "Arduino.h"
#include <Wire.h>
#include <RTClib.h>
#include <Time.h>
RTC_DS1307 RTC;

#define debugging 0

// HIGH signal bedeutet Kontakt geschlosssen
#define doorCloseSensePin  11
#define doorCloseLedPin  9
// HIGH signal bedeutet Kontakt geschlosssen
#define doorOpenSensePin   12
#define doorOpenLedPin  10

#define doorControlPin  2

// Relais-Pin
#define observeControlPin  13

int doorOpenState = -1;
int doorCloseState = -1;
unsigned long int closeTime ;
bool doorObserve = false;

// in seconds
#define observeTime 20// 10 Minuten

int ledState = LOW;
unsigned long int  ledStateChange;
int  ledOnTime ;
int ledOffTime ;
int writeCounter = 0;

int currentObservableTime = -1; //0: nein, 1: ja; -1: undefined

void setup() {
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();

  // initialize digital pins
  pinMode(doorOpenSensePin, INPUT_PULLUP);
  pinMode(doorCloseSensePin, INPUT_PULLUP);
  pinMode(doorOpenLedPin, OUTPUT);
  pinMode(doorCloseLedPin, OUTPUT);

  pinMode(doorControlPin, OUTPUT);
  pinMode(observeControlPin, OUTPUT);
  digitalWrite(doorControlPin, HIGH);
  // keep initially the pin states of the doorPins
  doorCloseState = digitalRead(doorCloseSensePin);
  doorOpenState = digitalRead(doorOpenSensePin);
  Serial.println("Garage ");



  if (! RTC.isrunning()) {
    Serial.println("set time ");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  writeTime(RTC.now());
}

void loop() {
  DateTime now = RTC.now();
  // wenn sich die observable time aendert
  int newObservableTime = 0;
  if (isObservableTime(RTC.now())) {
    newObservableTime = 1;
  }

  if (newObservableTime != currentObservableTime) {
    // wechsel von observabl nach not observable oder umgekehrt
    doorOpenState = -1;
    doorCloseState = -1;
    currentObservableTime = newObservableTime;
    if (currentObservableTime == 1) {
      writeTime(RTC.now());
      Serial.write("switch to observable time\n");
      setBlinkRatio(10000, 0);
    } else {
      writeTime(RTC.now());
      Serial.write("switch to NON-observable time\n");
      setBlinkRatio(0, 10000);
    }
  }

  if (currentObservableTime == 1) {
    doorOpen();
    doorClosed();
    if (doorObserve) {
      Serial.write('.');
      writeCounter++;
      if (writeCounter > 50) {
        writeCounter = 0 ;
        Serial.write("\n");
        Serial.write("closing in ");
        unsigned long int remTime =  closeTime - now.unixtime() ;
        char buf[9];
        sprintf(buf, "%04u", remTime);
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

  checkLedState(millis());

  delay(50);
}

void setBlinkRatio(int on, int off) {
  // stop current blinking;
  Serial.print("blink: on=");
  Serial.print(on);
  Serial.print(", off=");
  Serial.println(off);
  digitalWrite(observeControlPin, HIGH);
  ledState = HIGH;
  ledStateChange = millis() + on;
  ledOnTime = on;
  ledOffTime = off;
}

void checkLedState(unsigned long  currTime) {
  if (currTime >= ledStateChange ) {
    if (ledState == HIGH) {
      digitalWrite(observeControlPin, LOW);
      ledStateChange = currTime + ledOffTime;
      ledState = LOW;
    } else {
      digitalWrite(observeControlPin, HIGH);
      ledStateChange = currTime + ledOnTime;
      ledState = HIGH;
    }
  }
}


boolean doorClosed() {
  int newState = digitalRead(doorCloseSensePin);
  digitalWrite(doorCloseLedPin, newState);
  if (newState != doorCloseState ) {
    writeTime(RTC.now());
    if (newState == LOW) {
      Serial.println("door down");
    }
    else {
      Serial.println("door moving up");
    }
    doorCloseState = newState;
  }
  return (newState == LOW);
}

boolean doorOpen() {
  int newState = digitalRead(doorOpenSensePin);
  digitalWrite(doorOpenLedPin, newState);
  if (newState != doorOpenState ) {
    writeTime(RTC.now());
    if (newState == LOW) {
      Serial.println("door top");
      if (isObservableTime(RTC.now())) {
        setBlinkRatio(800, 800);

        startObserve();
      }
    }
    else {
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
  DateTime dt7 (closeTime);
  writeTime( dt7);

}

void stopObserve() {
  Serial.println("stop observing");
  doorObserve = false;
  setBlinkRatio(10000, 0);
}

void closeDoor() {
  doorObserve = false;
  Serial.println("close door now");
  digitalWrite(doorControlPin, LOW);
  delay(1000);
  setBlinkRatio(100, 100);

  while (doorOpen()) {
    delay(1000);
    Serial.println(" door not moving");
    checkLedState(millis());
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



