#include <GC.h>
#include <Servo.h>

GC_Controller gc(3);
Servo serv;
int prev_angle = 1;
bool calibrated_1 = false;
bool calibrated_2 = false;
bool good = false;
byte left = 0;
byte right = 0;

void setup() {
  Serial.begin(1000000);
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW);
  serv.attach(9);
  serv.write(90);
}

void loop() {
  
  gc.update(); //better if in an interupts
  
  if (!gc.is_connected()) {
    Serial.println("Please connect controller");
  }
  else {
    if (!calibrated_1) {
      Serial.println("Put stick all the way to the left, then press A");
      if (gc.A()) {
        left = gc.JOY_X();
        calibrated_1 = true;
      }
    }
    if (calibrated_1 && !calibrated_2 && !gc.A()) good = true;
    if (calibrated_1 && !calibrated_2 && good) {
      Serial.println("Put stick all the way to the right, then press A");
      if (gc.A()) {
        right = gc.JOY_X();
        calibrated_2 = true;
      }
    }
    if (calibrated_1 && calibrated_2) {
      int angle = map(gc.JOY_X(), left, right, 10, 170);
      if (abs((angle - prev_angle)*100/prev_angle) > 2) {
        serv.write(angle);
        Serial.println(angle);
        prev_angle = angle;
      }
      
    }
  }
  delay(30);
}
