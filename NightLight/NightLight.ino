#include <avr/sleep.h>    // Sleep Modes
#include <avr/power.h>    // Power management

const uint8_t ledPin = 0;
const uint8_t buttonPin = 2;
const uint8_t ledTimeMultiplier = 60;

const unsigned long buttonInterval = 200;
const unsigned long ledInterval = (unsigned long) (600 * 1000.0 /255.0);

unsigned long buttonPressedAt = 0;
unsigned long lastButtonCheck = 0;
unsigned long lastLedCheck = 0;
unsigned long ledTime;
unsigned long buttonReleasedAt;
boolean buttonPressedLastRead = false;
int ledIntensity = 100;

void setup() {
  /* Serial.begin(57600); */
  /* Serial.print("led interval "); Serial.println(ledInterval); */
  pinMode(ledPin, OUTPUT);      
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop(){
  unsigned long currentMillis = millis();
  
  if((unsigned long)(currentMillis - buttonReleasedAt) > ledTime && ledIntensity > 0 && !buttonPressedLastRead) {
      if((unsigned long)(currentMillis - lastLedCheck) > ledInterval) {
        lastLedCheck = currentMillis;
        ledIntensity--;
        /* Serial.print("intensity: "); Serial.println(ledIntensity); */
      }
  }
  analogWrite(ledPin, ledIntensity);
  if((unsigned long)(currentMillis - lastButtonCheck) > buttonInterval) {
    lastButtonCheck = currentMillis;
    if(digitalRead(buttonPin)) {
      /* Serial.println("HIGH"); */
      if(buttonPressedLastRead) {
        buttonReleasedAt = currentMillis;
        ledTime = (unsigned long)(buttonReleasedAt - buttonPressedAt) * ledTimeMultiplier;
        /* ledIntensity = 255; */
      }
      buttonPressedLastRead = false;
    } else {
      /* Serial.println("LOW"); */
      if(!buttonPressedLastRead) {
        buttonPressedAt = currentMillis;
        ledIntensity = 255;
        ledTime = 0;
      }
      buttonPressedLastRead = true;
    }
  }
  if(ledIntensity == 0) {
    /* loop_until_bit_is_set(UCSR0A, TXC0); /\* Wait until transmission ready. *\/ */
    sleepNow();
  }
}

void wake () {
  sleep_disable ();         // first thing after waking from sleep:
  detachInterrupt (0);      // stop LOW interrupt
  /* Serial.println("wake"); */
}

void sleepNow () {
  /* Serial.println("sleeping"); */
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  noInterrupts ();          // make sure we don't get interrupted before we sleep
  sleep_enable ();          // enables the sleep bit in the mcucr register
  attachInterrupt (0, wake, LOW);  // wake up on low level
  interrupts ();           // interrupts allowed now, next instruction WILL be executed
  sleep_cpu ();            // here the device is put to sleep
  power_all_enable();    // power everything back on
}
