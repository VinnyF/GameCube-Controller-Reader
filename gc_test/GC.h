#ifndef _GC_Controller_h_
#define _GC_Controller_h_

class GC_Controller {

  public:

    //Constructor
    GC_Controller(int input_pin,int output_pin): in(input_pin), out(output_pin) {}
    
    void ping_controller();
    void probe_controller();
    void read_controller();
    void poll_controller();
    byte* raw_data() {return data;}
    
  private:
  
    int in; //Input/Output pins hardcoded for speed, revisit later
    int out;
    byte data[8];

    void delay_l1();
    void delay_h3();
    void delay_l3();
    void delay_h1();
    
    void send_byte(byte data);
    void send_stop();
    
    byte read_byte();
  
};

//Hold the line low for ~1us
void GC_Controller::delay_l1()
{
  asm volatile (
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}

//Hold the line high for ~3us
void GC_Controller::delay_h3()
{
  asm volatile (
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}

//Hold the line low for ~3us
void GC_Controller::delay_l3()
{
  asm volatile (
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}

//Hold the line high for ~1us
void GC_Controller::delay_h1()
{
  asm volatile (
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}

//Writes a byte to the data line according to the
//GC protocol
void GC_Controller::send_byte(byte data) {

  //Start loop with mask = 1000 0000
  //Perform bitwise AND with the byte passed in
  //This will reveal the value of each individual byte,
  //starting with the MSB
  //Result will be 0 if the bit is 0, non-zero if the bit is 1
  //After, shift the mask right to move to the next bit.
  for (byte mask = 0x80; i > 0; i >>= 1) {

    //To send a 1, write a 0 for 1 us, followed by 1 for 3us
    //To send a 0, write a 0 for 3 us, followed by 1 for 1us
    
    if (data & mask) { //Bit is 1
      PORTD &= 0xF7; //Pull low
      delay_l1(); //Hold low for 1us
      PORTD |= 0x08; //Pull high
      delay_h3(); //Hold high for 3us
    }
    else { //Bit is 0
      PORTD &= 0xF7; //Pull low
      delay_l3(); //Hold low for 3us
      PORTD |= 0x08; //Pull high
      delay_h1(); //Hold high for 1us
    }
  }
}

//A stop bit is a single 1 bit
//Code is identical to sending a 1
void GC_Controller::send_stop() {
  PORTD &= 0xF7;
  delay_l1();
  PORTD |= 0x08;
  delay_h3();
}

//Currently not working
byte GC_Controller::read_byte() {

  //Build a byte by reading data serially,
  //Taking in a 0 or a 1, then shifting it left
  
  byte reading = 0;
  for (int i = 0; i < 8; i++) {
    //while (PIND & 0x08);
    //delay2();
    if (PIND & 0x08) reading++;
    if (i < 7) reading <<= 1;
    delay_l3();
  }
  return reading;
}

//Send the 24-bit sequence and stop bit
//to force the controller to respond.
void GC_Controller::ping_controller() {
  byte byte0 = 0x40; //0100 0000
  byte byte1 = 0x03; //0000 0011
  //byte byte2 = 0x02; //0000 0010
  byte byte2 = 0x03; //0000 0011
  send_byte(byte0);
  send_byte(byte1);
  send_byte(byte2);
  send_stop();
}

//Send a byte of 0s and stop bit to 
//prompt a response from the controllers
void GC_Controller::probe_controller() {
  noInterrupts();
  byte byte0 = 0xFF;
  //byte byte0 = 0x00;
  send_byte(byte0);
  send_stop();
  interrupts();
}

//Not working
void GC_Controller::read_controller() {
  for (int i = 0; i < 8; i++) {
    data[i] = read_byte();
  }
}

//The complete sequence of pinging the controller
//and reading it. Not currently working
void GC_Controller::poll_controller() {
  noInterrupts();
  DDRD |= 0x08;
  ping_controller();
  DDRD &= ~0x08;
  read_controller();
  interrupts();
}

#endif
