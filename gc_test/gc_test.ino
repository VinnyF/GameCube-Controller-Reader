#include <GC.h>

GC_Controller gc(3);
unsigned long long l_data;
byte data[8];

void setup() {
  Serial.begin(1000000);
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW);
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
