/*
GC.h
By Vincent Ferrara
Created May 2017

Use a GameCube controller with Arduino projects!

See the GitHub wiki for full documentation
https://github.com/VinnyF/GameCube-Controller-Reader
*/

#ifndef _GC_Controller_h_
#define _GC_Controller_h_

//Identifiers for the type of Arduino board
#define UNO 1
#define MEGA 2

#include "Arduino.h"

class GC_Controller {

  public:

    //Constructor
    GC_Controller(const int pin_, const int board_ = UNO);
    
    //Polls and reads the controller, then updates the data
    void update();

    //Checks if a controller is connected
    bool is_connected() const {return data != 0xFFFFFFFFFFFFFFFF;};

    //Returns the entire 64-bit number of data
    unsigned long long raw_data() const {return data;}

    //Convenient functions for checking state of buttons
    bool START()    const {return buttons[0];}
    bool Y()        const {return buttons[1];}
    bool X()        const {return buttons[2];}
    bool B()        const {return buttons[3];}
    bool A()        const {return buttons[4];}
    bool L()        const {return buttons[6];}
    bool R()        const {return buttons[7];}
    bool Z()        const {return buttons[8];}
    bool D_UP()     const {return buttons[9];}
    bool D_DOWN()   const {return buttons[10];}
    bool D_RIGHT()  const {return buttons[11];}
    bool D_LEFT()   const {return buttons[12];}

    //Convenient functions for getting analog data
    byte JOY_X()      const {return jx;}
    byte JOY_Y()      const {return jy;}
    byte C_X()        const {return cx;}
    byte C_Y()        const {return cy;}
    byte L_TRIGGER()  const {return lt;}
    byte R_TRIGGER()  const {return rt;}
    
    //Functions to set deadzones for the analog sticks
    void set_j_deadzone(const unsigned int dzone);
    void set_c_deadzone(const unsigned int dzone);

  private:
  
    byte pin_mask;                //Used to control which pin the controller is connected to
    int board;                    //Number representing the type of Arduino board being used
    long long data;               //Initiallize 64 bits of data
    unsigned char j_deadzone = 0; //Deadzone threshold for the gray joystick
    unsigned char c_deadzone = 0; //Deadzone threshold for the c-stick

    //Initiallize each piece of relevant controller data
    bool buttons[13]; //Holds every digital button state
    byte jx = 0;      //Joystick X value
    byte jy = 0;      //Joystick Y value
    byte cx = 0;      //C-Stick X value
    byte cy = 0;      //C-Stick Y value
    byte lt = 0;      //Left trigger value
    byte rt = 0;      //Right trigger value

    //Functions related to reading the controller
    void ping();                  //Sends a 24-bit sequence
    unsigned long long read();    //Reads data coming in from the controller
    void poll();                  //Pings and read the controller
    void update_buttons();        //Unpacks the 64-bit data number and updates the object variables appropriately

    //DELAYS
    void delay_l1(); //Hold low for ~1us
    void delay_h3(); //Hold high for ~3us
    void delay_l3(); //Hold low for ~3us
    void delay_r0(); //Wait ~4us after 0 has been read
    void delay_r1(); //Wait ~4us after 1 has been read
    void delay_offset(); //Align the reading point for accuracy

    //Modified functions for use with an Arduino Mega
    void poll_m();                //Pings and read the controller
    unsigned long long read_m();  //Reads data coming in from the controller
    void delay_r0_m();            //Wait ~4us after 0 has been read
    void delay_r1_m();            //Wait ~4us after 1 has been read
    void delay_offset_m();        //Align the reading point for accuracy
};

//Returns true if the analog value is within the set deadzone threshold
bool within_deadzone(unsigned char x, unsigned char y, unsigned char dzone);

#endif