/*

* Changeable.h

*

*  Created on: 14.07.2016

*      Author: frankp

*/



#ifndef CHANGEABLE_H_

#define CHANGEABLE_H_



class Changeable {



public:

       Changeable(int startValue);

       virtual ~Changeable();



       void setValue(int newValue);

       int getValue();



       void (*myCallback)(int);

       void setCallback(void (*callback)(int))

       {

              myCallback = callback;

       }

private :

       int value;

};



#endif /* CHANGEABLE_H_ */
