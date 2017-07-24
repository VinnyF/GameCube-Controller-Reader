/*
gc_test.ino
By Vincent Ferrara
https://github.com/VinnyF/GameCube-Controller-Reader

This test will read the controller about ever 30ms.
After each reading, the entire state of the controller
it output to the serial monitor.
*/

#include <GC.h>

GC_Controller gc(3);

void setup() {
  Serial.begin(500000);
}

void loop() {
  
  gc.update();

  Serial.print("A: ");
  Serial.println(gc.A());
  Serial.print("B: ");
  Serial.println(gc.B());
  Serial.print("X: ");
  Serial.println(gc.X());
  Serial.print("Y: ");
  Serial.println(gc.Y());
  Serial.print("Z: ");
  Serial.println(gc.Z());
  Serial.print("L: ");
  Serial.println(gc.L());
  Serial.print("R: ");
  Serial.println(gc.R());
  Serial.print("START: ");
  Serial.println(gc.START());
  Serial.print("D_UP: ");
  Serial.println(gc.D_UP());
  Serial.print("D_DOWN: ");
  Serial.println(gc.D_DOWN());
  Serial.print("D_LEFT: ");
  Serial.println(gc.D_LEFT());
  Serial.print("D_RIGHT: ");
  Serial.println(gc.D_RIGHT());
  Serial.print("JOY_X: ");
  Serial.println(gc.JOY_X());
  Serial.print("JOY_Y: ");
  Serial.println(gc.JOY_Y());
  Serial.print("C_X: ");
  Serial.println(gc.C_X());
  Serial.print("C_Y: ");
  Serial.println(gc.C_Y());
  Serial.print("L_TRIGGER: ");
  Serial.println(gc.L_TRIGGER());
  Serial.print("R_TRIGGER: ");
  Serial.println(gc.R_TRIGGER());

  Serial.println("BREAK");
  delay(30);
}
