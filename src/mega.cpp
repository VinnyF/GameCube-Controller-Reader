//This file contains slightly modified versions of time-sensitive functions
//meant to work with an Arduino Mega

#include "GC.h"

unsigned long long GC_Controller::read_m() {

  unsigned long long reading = 0;

  //Start the mask at 1000 0000 ..., then shift right
  for (unsigned long long mask = 0x8000000000000000; mask > 0; mask >>= 1) {

    if (PIND & pin_mask) { //Reading a 1

      //Instead of adding and shifting, simply use bitwise OR
      //Presumably, it is faster
      reading |= mask;

      delay_r1_m(); //Wait remaining ~4us
    }
    else //Read a 0
      delay_r0_m(); //Wait remaining ~4us

    //The OR operation takes a certain amount of time,
    //so the delay to the next bit must be different
    //depending on whether or not the OR had to happen
  }
  return reading;
}

void GC_Controller::poll_m() {

  noInterrupts(); //Disabling interrupts will increase accuracy

  DDRD |= pin_mask; //Changes pin to output mode
  ping(); //Output 24-bit sequence

  //Changes pin to input mode
  //Necessary for reading as well as not interfering with the incoming data
  DDRD &= ~pin_mask;

  delay_offset_m(); //Wait a moment to align the reading
  data = read_m(); //Read the data coming in

  interrupts(); //Interrupts are OK past this point
}