/*
 */
//#include "printf.h"
#include <avr/sleep.h>    // Sleep Modes
#include <avr/power.h>    // Power management

const uint8_t buttonPin[] = {2, 4, 6, 8};
const uint8_t ledPin[] =  {3, 5, 7, 9};
const unsigned long timeout=1000*30;

unsigned long lastButtonPress;
uint8_t buttonState[] = {1, 1, 1, 1};
uint8_t previousButtonState[] = {0, 0, 0, 0};
uint8_t randomSequence[100];
uint8_t turn = 0;
uint8_t current = 0;
uint8_t temp1,temp2;
boolean fail = false;

void setup() {
//  Serial.begin(57600);
//  Serial.println("Setup");
  randomSeed(analogRead(0));
  for(int i = 0 ; i < 4; i++) {
    pinMode(ledPin[i], OUTPUT);      
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  lastButtonPress = millis();
}

void loop(){
  unsigned long currentMillis = millis();
  if((unsigned long)(currentMillis - lastButtonPress) > timeout) {
    failGame();
    sleepNow();
    fail = true; //Init game and blink
    lastButtonPress = millis();
  }
  if(fail) {
//    Serial.println("fail");
    turn = 0;
    current = 0;
    fail = false;
    failGame();
  } else if (current == turn) {
    if(turn == 0) {
//      Serial.println("start game");
      initializeGame();
//      startGame();
    }
    turn++;
    current = 0;
//    Serial.println("play random sequence");
    playRandomSequence(turn);
  } else {
    for(int i = 0; i < 4; i++) {
      temp1 = digitalRead(buttonPin[i]);
      delay(10);
      temp2 = digitalRead(buttonPin[i]);
      if(temp1 == temp2) {
        previousButtonState[i] = buttonState[i];
        buttonState[i] = temp1;
      
        // check if the push button is pressed.
        // if it is, the buttonState is LOW:
        if (buttonState[i] == LOW) {
          lastButtonPress = currentMillis;
          // turn LED on:    
          digitalWrite(ledPin[i], HIGH);  
        } else {
          // turn LED off:
          digitalWrite(ledPin[i], LOW);
          if(previousButtonState[i] == LOW) {
            //button released
            if(randomSequence[current] == i) {
              current++;
            } else {
              fail = true;
            }
          }
        }
      }
    }
  }
}

void playRandomSequence(int howMany) {
  allOff();
  delay(500);
  for(int i=0; i < howMany; i++) {
    digitalWrite(ledPin[randomSequence[i]], HIGH);
    delay(1000);
    digitalWrite(ledPin[randomSequence[i]], LOW);
    delay(300);
  }
}

void initializeGame() {
  for(int i = 0; i < 100; i++) {
    randomSequence[i] = random(0, 4);
  }
}

void startGame() {
  sequence(10);
}
void sequence(int times) {
  for(int i = 0; i<times; i++) {
    allOff();
    delay(100);
    for(int i=0; i<4;i++) {
      digitalWrite(ledPin[i], HIGH);
      delay(100);
    }
  }
}

void failGame() {
  blink(3);
}

void blink(int times) {
  allOff();
  for(int i = 0; i<times; i++) {
    allOn();
    delay(600);
    allOff();
    delay(400);
  }
}
void allOn() {
  for(int i=0; i<4;i++) {
    digitalWrite(ledPin[i], HIGH);
  }
}
void allOff() {
  for(int i=0; i<4;i++) {
    digitalWrite(ledPin[i], LOW);
  }
}

void wake () {
  sleep_disable ();         // first thing after waking from sleep:
  detachInterrupt (0);      // stop LOW interrupt
}  // end of wake
void sleepNow () {
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  noInterrupts ();          // make sure we don't get interrupted before we sleep
  sleep_enable ();          // enables the sleep bit in the mcucr register
  attachInterrupt (0, wake, LOW);  // wake up on low level
  interrupts ();           // interrupts allowed now, next instruction WILL be executed
  sleep_cpu ();            // here the device is put to sleep
  power_all_enable();    // power everything back on
}  // end of sleepNow

