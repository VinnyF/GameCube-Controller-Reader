/*
For use with the GCReader test application
https://github.com/VinnyF/GameCube-Controller-Reader/wiki
*/

#include <GC.h>

//Initialize controller and buffer
GC_Controller gc(3);
byte buffer[8];

void setup() {
  Serial.begin(512000);
  gc.set_j_deadzone(10);
  gc.set_c_deadzone(10);
}

void loop() {

  //Listen for ping from program
  while (Serial.read() != 'A');

  //After ping, read the controller
  gc.update();
  unsigned long long data = gc.raw_data();

  //Package data into buffer
  buffer[0] = data >> 56;
  buffer[1] = data >> 48;

  //Don't get the raw data in order to preserve deadzone
  buffer[2] = gc.JOY_X();
  buffer[3] = gc.JOY_Y();
  buffer[4] = gc.C_X();
  buffer[5] = gc.C_Y();
  
  buffer[6] = data >> 8;
  buffer[7] = data;

  //Send data
  Serial.write(buffer, 8);
}
