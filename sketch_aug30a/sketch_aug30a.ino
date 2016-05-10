#include <CapacitiveSensor.h>

//CapacitiveSensor   cs_2_8 = CapacitiveSensor(2,8);        // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
CapacitiveSensor   cs_2_9 = CapacitiveSensor(2,9);        // 10 megohm resistor between pins 4 & 6, pin 6 is sensor pin, add wire, foil


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
    long start = millis();
    //long total1 =  cs_2_8.capacitiveSensor(30);
    long total2 =  cs_2_9.capacitiveSensor(30);
   
    Serial.print(millis() - start);        // check on performance in milliseconds
    Serial.print("\t");                    // tab character for debug window spacing

 //   Serial.print(total1);                  // print sensor output 1
 //   Serial.print("\t");
    Serial.println(total2);                  // print sensor output 2
 
    delay(1000);                             // arbitrary delay to limit data to serial port
}
