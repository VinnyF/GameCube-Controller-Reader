#include <GC.h>

GC_Controller gc(3);
byte buffer[8];

void setup() {
  Serial.begin(512000);
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW);
}

void loop() {

  while (Serial.read() != 'A');
  
  gc.update();
  unsigned long long data = gc.raw_data();

  buffer[0] = data >> 56;
  buffer[1] = data >> 48;
  buffer[2] = data >> 40;
  buffer[3] = data >> 32;
  buffer[4] = data >> 24;
  buffer[5] = data >> 16;
  buffer[6] = data >> 8;
  buffer[7] = data;
  
  Serial.write(buffer, 8);
}
