#include "GC.h"

GC_Controller gc(2,3);

void setup() {
  Serial.begin(19200);
  pinMode(3,OUTPUT);

  digitalWrite(3,LOW);
}

void loop() {
  gc.poll_controller();
  byte* data;
  data = gc.raw_data();
  for (int i = 0; i < 8; i++) {
    Serial.println(data[i]);
  }
  Serial.println("BREAK");
  delay(3000);
}
