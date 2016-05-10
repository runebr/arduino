#include <NexaCtrl.h>

#define TX_PIN 6
#define RX_PIN 8
#define sign(x) ( (x>=0) ? +1 : -1 )

const static unsigned long controller_id = 10516222;
unsigned int device = 0;
boolean state = true;

NexaCtrl nexaCtrl(TX_PIN, RX_PIN);

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(A0, INPUT);
}
void loop() {
  /* int amp = analogRead(0); */
  /* Serial.println(amp); */
  /* delay(100); */
  waitForClap();
  if(state) {
    nexaCtrl.DeviceOn(controller_id, device);
    digitalWrite(13, HIGH);
  } else {
    nexaCtrl.DeviceOff(controller_id, device);
    digitalWrite(13, LOW);
  }
  state = !state;
}

void waitForClap() {
  bool clap = false;
  int zeroCrossings=0;
  int highAmp=0;
  int maxAmp = 0;
  do {
    int amp,prev_amp;
  
    prev_amp = analogRead(0) - 512;
  
    zeroCrossings=0;
    highAmp=0;
  
    // I estimate this loop will go about 8KHz, thus 80 times is 10 ms
  
    for (int i=0; i<=160; i++) {
      amp = analogRead(0) - 512;
      if (sign(amp) != sign(prev_amp)) zeroCrossings++;
      if (abs(amp) > 40) highAmp++;
      if(abs(amp) > maxAmp) maxAmp = amp;
    }
    
    //Serial.print("prev_amp: "); 
    //Serial.println(prev_amp);
    clap = (highAmp > 10 && zeroCrossings > 20); // all numbers subject to experimentation!

  } while (!clap);
    Serial.print("zeroCrossings: ");
    Serial.println(zeroCrossings);
    Serial.print("highamp: ");
    Serial.println(highAmp);
    Serial.print("maxAmp: ");
    Serial.println(maxAmp);
    delay(20); //debounce
}

