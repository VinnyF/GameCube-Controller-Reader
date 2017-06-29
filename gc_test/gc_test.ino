#include "GC.h"

GC_Controller gc(2,3);

volatile void delay1(uint32_t us)
{
    while (--us){
        asm (
        " NOP\n\t"
        );
    }
}

volatile void delay2(uint32_t us)
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

void setup() {
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);

  digitalWrite(3,LOW);
  digitalWrite(2,LOW);
}

void loop() {

  /*
  PORTD |= 0x08;
  delay2(1);
  PORTD &= 0xF7;
  delay2(3);
 */

  gc.ping_controller();
  delay(12);
  /*
  delay(1500);
  Serial.println("Break");
  

  gc.probe_controller();
  //delayMicroseconds(15);
  gc.read_controller();
  
  byte* readings;
  readings = gc.raw_data();
  for(int i = 0; i < 8; i++) {
    Serial.println(readings[i]);
  }
  */

  
}
