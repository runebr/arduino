//#include <SoftwareSerial.h>
#include <CapacitiveSensor.h>

CapacitiveSensor   cs_2_3 = CapacitiveSensor(2, 3);       // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
CapacitiveSensor   cs_2_4 = CapacitiveSensor(2, 4);       // 10 megohm resistor between pins 4 & 6, pin 6 is sensor pin, add wire, foil
//SoftwareSerial debug(10, 1);

int ledPin = 0;
int ledIntensity = 0;
unsigned long lastChange = 0;
unsigned long delayMillis = 20;
int inc = 1;

void setup() {
  pinMode(ledPin, OUTPUT);
  //  debug.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  analogWrite(ledPin, ledIntensity);
  long total1 =  cs_2_3.capacitiveSensor(30);
  long total2 =  cs_2_4.capacitiveSensor(30);
  //debug.print(total1);
  //debug.print(" ");
  //debug.print(total2);
  //debug.print(" ");
  //debug.println(ledIntensity);
  if ((total1 > 25 || total2 > 20) && (unsigned long)(currentMillis - lastChange) > delayMillis) {
    if (total1 > total2 ) {
      ledIntensity += inc;
      if (ledIntensity > 100) ledIntensity = 100;
    } else {
      ledIntensity -= inc;
      if (ledIntensity < 0) ledIntensity = 0;
    }
    lastChange = currentMillis;
  }
}
