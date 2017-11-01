/*
 * BlinkLed.h
 *
 *  Created on: 19.08.2017
 *      Author: Frank
 */

#ifndef BLINKLED_H_
#define BLINKLED_H_
#include "Arduino.h"

class BlinkLed {
  private:
    int ledOnTime = 0;
    int ledOffTime = 0;
    int ledPin = 0;
    int ledState = LOW;
    char *name;
    unsigned long int ledStateChange = 0;

  public:
    BlinkLed();
    virtual ~BlinkLed();
    void check();
    void begin(int pin, const char *name);
    void setBlinkRatio(int on, int off);
};
#endif /* BLINKLED_H_ */

