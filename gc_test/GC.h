#ifndef _GC_Controller_h_
#define _GC_Controller_h_

class GC_Controller {

  public:

    //Constructor
    GC_Controller(int input_pin,int output_pin): in(input_pin), out(output_pin) {}
    
    //Polls and reads the controller, then updates the data
    void update();

    //Checks if a controller is connected
    bool is_connected();

    //Returns the entire 64-bit number of data
    unsigned long long raw_data() {return data;}

    //Convenient functions for checking state of buttons
    bool START() {return buttons[0];}
    bool Y() {return buttons[1];}
    bool X() {return buttons[2];}
    bool B() {return buttons[3];}
    bool A() {return buttons[4];}
    bool L() {return buttons[6];}
    bool R() {return buttons[7];}
    bool Z() {return buttons[8];}
    bool D_UP() {return buttons[9];}
    bool D_DOWN() {return buttons[10];}
    bool D_RIGHT() {return buttons[11];}
    bool D_LEFT() {return buttons[12];}

    //Convenient functions for getting analog data
    byte JOY_X() {return jx;}
    byte JOY_Y() {return jy;}
    byte C_X() {return cx;}
    byte C_Y() {return cy;}
    byte L_TRIGGER() {return lt;}
    byte R_TRIGGER() {return rt;}
    
  private:
  
    int in; //Input/Output pins hardcoded for speed, revisit later
    int out;
    long long data; //Initiallize 64 bits of data

    //Initiallize each piece of relevant controller data
    bool buttons[13];
    byte jx = 0;
    byte jy = 0;
    byte cx = 0;
    byte cy = 0;
    byte lt = 0;
    byte rt = 0;  

    void probe(); //Sends a byte of 0s
    void ping(); //Sends a 24-bit sequence
    unsigned long long read(); //Reads data coming in from the controller
    void poll(); //Pings and read the controller
    void update_buttons(); //Unpacks the 64-bit data number and updates the object variables appropriately

    //DELAYS
    void delay_l1(); //Hold low for ~1us
    void delay_h3(); //Hold high for ~3us
    void delay_l3(); //Hold low for ~3us
    void delay_h1(); //Hold high for ~1us
    void delay_r0(); //Wait ~4us after 0 has been read
    void delay_r1(); //Wait ~4us after 1 has been read
    void delay_offset(); //Align the reading point for accuracy
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
  );
}

//Wait ~4us after reading a low bit
void GC_Controller::delay_r0()
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
  );
}

//Wait ~4us after reading a high bit
void GC_Controller::delay_r1()
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
  );
}

//Aligns the point the data is read
//at the center of the 4us bit.
//This helps make the reading accurate.
void GC_Controller::delay_offset()
{
  asm volatile (
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
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
      PORTD &= ~0x08; //Pull low
      delay_l1(); //Hold low for 1us
      PORTD |= 0x08; //Pull high
      delay_h3(); //Hold high for 3us
    }
    else { //Bit is 0
      PORTD &= ~0x08; //Pull low
      delay_l3(); //Hold low for 3us
      PORTD |= 0x08; //Pull high
      delay_h1(); //Hold high for 1us
    }
  }
}

//Reads all 64 bits of the controller's response at once
unsigned long long GC_Controller::read() {

  unsigned long long reading = 0;

  //Start the mask at 1000 0000 ..., then shift right
  for (unsigned long long mask = 0x8000000000000000; mask > 0; mask >>= 1) {

    if (PIND & 0x08) { //Reading a 1

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

  DDRD |= 0x08; //Changes pin to output mode
  ping(); //Output 24-bit sequence

  //Changes pin to input mode
  //Necessary for reading as well as not interfering with the incoming data
  DDRD &= ~0x08;

  delay_offset(); //Wait a moment to align the reading
  data = read(); //Read the data coming in

  interrupts(); //Interrupts are OK past this point
}

//Send a byte of 0s and stop bit to 
//prompt a response from the controller
void GC_Controller::probe() {

  //For sake of consistent timing, this function
  //is taken from ping(). The bit masking and
  //if statement are completely unneccesary,
  //but are there to keep the right delays.
  unsigned long sequence = 0x00;

  for (byte mask = 0x80; mask > 0; mask >>= 1) {

    //To send a 1, write a 0 for 1 us, followed by 1 for 3us
    //To send a 0, write a 0 for 3 us, followed by 1 for 1us
    
    if (!(sequence & mask)) { //Bit is 0
      PORTD &= ~0x08; //Pull low
      delay_l3(); //Hold low for 3us
      PORTD |= 0x08; //Pull high
      delay_h1(); //Hold high for 1us
    }
  }
  //Send a high stop bit
  PORTD &= ~0x08; //Pull low
  delay_l1(); //Hold low for 1us
  PORTD |= 0x08; //Pull high
  delay_h3(); //Hold high for 3us
}

//Probes the controller
//Returns true if any data is sent back
bool GC_Controller::is_connected() {

  noInterrupts(); //Disabling interrupts will increase accuracy
  DDRD |= 0x08; //Changes pin to output mode

  probe(); //Prompts a response from the controller

  //Changes pin to input mode
  //Necessary for reading as well as not interfering with the incoming data
  DDRD &= ~0x08;
  delay_offset(); //Align for reading data

  //Checks if any data has been recieved
  if (read() != 0xFFFFFFFFFFFFFFFF) {
    interrupts();
    return true;
  }
  interrupts();
  return false;
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
}

//Complete polling and updating sequence
void GC_Controller::update() {
  poll();
  update_buttons();
}

#endif
