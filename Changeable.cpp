/*

 * Changeable.cpp

 *

 *  Created on: 14.07.2016

 *      Author: frankp

 */


#include "Changeable.h"


Changeable::Changeable(int startValue) {

   value = startValue;

   myCallback = 0;

}

Changeable::~Changeable() {

}

void Changeable::setValue(int newValue) {

   if (value != newValue) {

       value = newValue;

      // call callback

      if (myCallback != 0) {

         (myCallback)(value);

      }

   }

}

int Changeable::getValue() {

   return value;

}

