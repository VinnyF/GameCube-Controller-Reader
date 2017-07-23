#include "GC.h"

//Constructor
//Sets the board number and pin mask
GC_Controller::GC_Controller(const int pin_, const int board_) {

  //Set the board number
  switch (board_) {
    case MEGA: board = MEGA; break;
    case UNO:
    default: board = UNO; break;
  }

  //Use the pin number to determine the pin mask
  switch (pin_) {
    case 21: //For Mega
    case 0: pin_mask = 0x01; break;
    case 20: //For Mega
    case 1: pin_mask = 0x02; break;
    case 19: //For Mega
    case 2: pin_mask = 0x04; break;
    case 18: //For mega
    case 3: pin_mask = 0x08; break;
    case 4: pin_mask = 0x10; break;
    case 5: pin_mask = 0x20; break;
    case 6: pin_mask = 0x40; break;
    case 7: pin_mask = 0x80; break;
    default: pin_mask = 0x00; break; //Shouldn't happen
  }
}

//Forces the controller to respond
//by sending the 24-bit polling sequence
//in GC protocol
void GC_Controller::ping() {

  //The 24 bit sequence: 0100 0000 0000 0011 0000 0010
  //The stop bit has been included at the end
  //The number is padded with 0s
  unsigned long sequence = 0x00800605;

  //Start loop with mask = 0000 0001 0000 0000 0000 0000 0000 0000
  //Perform bitwise AND with the sequence
  //This will reveal the value of each individual bit,
  //starting with the MSB
  //Result will be 0 if the bit is 0, non-zero if the bit is 1
  //After, shift the mask right to move to the next bit.
  for (unsigned long mask = 0x01000000; mask > 0; mask >>= 1) {

    //To send a 1, write a 0 for 1 us, followed by 1 for 3us
    //To send a 0, write a 0 for 3 us, followed by 1 for 1us
    
    if (sequence & mask) { //Bit is 1
      PORTD &= ~pin_mask; //Pull low
      delay_l1(); //Hold low for 1us
      PORTD |= pin_mask; //Pull high
      delay_h3(); //Hold high for 3us
    }
    else { //Bit is 0
      PORTD &= ~pin_mask; //Pull low
      delay_l3(); //Hold low for 3us
      PORTD |= pin_mask; //Pull high
      //Hold high for 1us: No delay needed
    }
  }
}

//Reads all 64 bits of the controller's response at once
unsigned long long GC_Controller::read() {

  unsigned long long reading = 0;

  //Start the mask at 1000 0000 ..., then shift right
  for (unsigned long long mask = 0x8000000000000000; mask > 0; mask >>= 1) {

    if (PIND & pin_mask) { //Reading a 1

      //Instead of adding and shifting, simply use bitwise OR
      //Presumably, it is faster
      reading |= mask;

      delay_r1(); //Wait remaining ~4us
    }
    else //Read a 0
      delay_r0(); //Wait remaining ~4us

    //The OR operation takes a certain amount of time,
    //so the delay to the next bit must be different
    //depending on whether or not the OR had to happen
  }
  return reading;
}

//The complete sequence of pinging the controller
//and immediately reading it.
void GC_Controller::poll() {

  noInterrupts(); //Disabling interrupts will increase accuracy

  DDRD |= pin_mask; //Changes pin to output mode
  ping(); //Output 24-bit sequence

  //Changes pin to input mode
  //Necessary for reading as well as not interfering with the incoming data
  DDRD &= ~pin_mask;

  delay_offset(); //Wait a moment to align the reading
  data = read(); //Read the data coming in

  interrupts(); //Interrupts are OK past this point
}

void GC_Controller::update_buttons() {

  //Unpacks the button section of the data into its own number
  //Should increase speed since the number is smaller
  unsigned int inputs = data >> 48;

  //Skip the first three bits
  unsigned int mask = 0x1000;

  //Checks the state of each bit, storing the result in an array
  for (char i = 0; i < 13; i++) {
    buttons[i] = inputs & mask;
    mask >>= 1;
  }

  //Unpacks each byte of analog data into its own variable
  jx = data >> 40;
  jy = data >> 32;
  cx = data >> 24;
  cy = data >> 16;
  lt = data >> 8;
  rt = data;

  //Check if analog values are within the deadzone threshold.
  if (within_deadzone(jx,jy,j_deadzone)) jx = jy = 128;
  if (within_deadzone(cx,cy,c_deadzone)) cx = cy = 128;
}

//Complete polling and updating sequence
void GC_Controller::update() {

  //Pick the right polling function based on the board type
  switch (board) {
    case UNO: poll(); break;
    case MEGA: poll_m(); break;
    default: poll(); break;
  }
  update_buttons();
}

//Set the deadzone. Make sure the value doesn't go above 120
void GC_Controller::set_j_deadzone(const unsigned int dzone) {
    if (dzone > 120) j_deadzone = 120;
    else j_deadzone = dzone;
}

//Set the deadzone. Make sure the value doesn't go above 120
void GC_Controller::set_c_deadzone(const unsigned int dzone) {
    if (dzone > 120) c_deadzone = 120;
    else c_deadzone = dzone;
}

//Returns true if both the x and y values are within the threshold
bool within_deadzone(unsigned char x, unsigned char y, unsigned char dzone) {
    return ((x < (128 + dzone) && x > (128 - dzone)) && 
            (y < (128 + dzone) && y > (128 - dzone)));
}