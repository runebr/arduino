/*
  Button
 
 Turns on and off a light emitting diode(LED) connected to digital  
 pin 13, when pressing a pushbutton attached to pin 2. 
 
 
 The circuit:
 * LED attached from pin 13 to ground 
 * pushbutton attached to pin 2 from +5V
 * 10K resistor attached to pin 2 from ground
 
 * Note: on most Arduinos there is already an LED on the board
 attached to pin 13.
 
 
 created 2005
 by DojoDave <http://www.0j0.org>
 modified 30 Aug 2011
 by Tom Igoe
 
 This example code is in the public domain.
 
 http://www.arduino.cc/en/Tutorial/Button
 */

// constants won't change. They're used here to 
// set pin numbers:
const int buttonPin[] = {12, 10, 8, 6};     // the number of the pushbutton pin
const int ledPin[] =  {13, 11, 9, 7};      // the number of the LED pin

// variables will change:
int buttonState[] = {0, 0, 0, 0};         // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  for(int i = 0 ; i < 4; i++) {
    pinMode(ledPin[i], OUTPUT);      
  // initialize the pushbutton pin as an input:
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  flash();
}

void loop(){
  for(int i = 0; i < 4; i++) {
    // read the state of the pushbutton value:
    buttonState[i] = digitalRead(buttonPin[i]);

    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (buttonState[i] == LOW) {     
      // turn LED on:    
      digitalWrite(ledPin[i], HIGH);  
    } else {
      // turn LED off:
      digitalWrite(ledPin[i], LOW); 
    }
  }
}

void flash() {
  sequence(10);
  blink(3);
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

