#ifndef _GC_Controller_h_
#define _GC_Controller_h_

class GC_Controller {

  public:

    //Constructor
    GC_Controller(int input_pin,int output_pin): in(input_pin), out(output_pin) {}
    
    void ping_controller();
    void probe_controller();
    void read_controller();
    byte* raw_data() {return data;}
    
  private:
  
    int in; //Input/Output pins hardcoded for speed, revisit later
    int out;
    byte data[64];

    volatile void delay1(uint32_t us);
    volatile void delay2(uint32_t us);
    
    void send_byte(bool* data);
    void send_stop();
    
    byte read_byte();
  
};

volatile void GC_Controller::delay1(uint32_t us)
{
    while (--us){
        asm (
        " NOP\n\t"
        );
    }
}

volatile void GC_Controller::delay2(uint32_t us)
{
    while (--us){
        asm (
        " NOP\n\t"
        " NOP\n\t"
        " NOP\n\t"
        " NOP\n\t"
        " NOP\n\t"
        " NOP\n\t"
        );
    }
}
//DISABLE INTERRUPTS
void GC_Controller::send_byte(bool* data) {
  
  for (int i = 0; i < 8; i++) {

    //To send a 0, write a 0 for 3 us, followed by 1 for 1us
    //To send a 1, write a 0 for 1 us, followed by 1 for 3us
    if (data[i]) {
      PORTD |= 0x08;
      delay2(2);
      PORTD &= 0xF7;
      delay2(1);
    }
    else {
      PORTD |= 0x08;
      //delay1(6);
      delay2(1);
      PORTD &= 0xF7;
      delay2(2);
    }
  }
}

void GC_Controller::send_stop() {
  PORTD |= 0x08;
  delay2(1);
  PORTD &= 0xF7;
  delay2(2);
}

byte GC_Controller::read_byte() {

  //Build a byte by reading data serially,
  //Taking in a 0 or a 1, then shifting it left
  
  byte reading = 0;
  for (int i = 0; i < 8; i++) {
    if (PIND & 0x04) reading++;
    if (i < 7) reading <<= 1;

    delayMicroseconds(4);
  }
  return reading;
}

void GC_Controller::ping_controller() {
  bool byte0[8] = {0,1,0,0,0,0,0,0};
  bool byte1[8] = {0,0,0,0,0,0,1,1};
  bool byte2[8] = {0,0,0,0,0,0,1,0};
  //bool byte2[8] = {0,0,0,0,0,0,1,1};
  send_byte(byte0);
  send_byte(byte1);
  send_byte(byte2);
  send_stop();
}

void GC_Controller::probe_controller() {
  bool byte0[8] = {0,0,0,0,0,0,0,0};
  send_byte(byte0);
  send_stop();
}

void GC_Controller::read_controller() {
  for (int i = 0; i < 64; i++) {
    data[i] = read_byte();
  }
}

#endif
