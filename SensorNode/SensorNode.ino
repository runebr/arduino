#include <avr/wdt.h>
#include <avr/sleep.h>
#include <SPI.h> 
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include "printf.h"

#define RF_SETUP 0x17
#define voltageFlipPin1 7
#define voltageFlipPin2 8

// Set up nRF24L01 radio on SPI pin for CE, CSN
RF24 radio(9,10);

// Example below using pipe5 for writing
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0x7365727631LL };

typedef struct{
  uint16_t counter;
  uint16_t tmp36;
  uint16_t moisture1;
  uint16_t moisture2;
  uint16_t moisture;
  uint16_t battery3V;
  uint16_t battery9V;
  boolean bucketEmpty;
} sensors_t;
typedef struct{
  uint16_t motor;
  uint16_t sleep;
} action_t;


sensors_t sensors;
action_t action;

int temperatureAnalogPin = A1;
int moistureAnalogPin = A2;
int battery3VAnalogPin = A3;
int battery9VAnalogPin = A4;
//int motorPin = 6;
int bucketEmptyPin = 5;

//int nrf24PowerPin = 4;

int counter=0;
int flipTimer = 1000;
int waitTime=5000;
int moistureLimit = 1024;
uint16_t nodeID = pipes[0] & 0xff;
unsigned long send_time;
volatile int watchdog_counter = 400;

void setup(void) {
  Serial.begin(57600);

  printf_begin();
  printf("Sending nodeID & 1 sensor data\n\r");
  //7mV for høy
  analogReference(INTERNAL);
  //pinMode(motorPin, OUTPUT);
  pinMode(voltageFlipPin1, OUTPUT);
  pinMode(voltageFlipPin2, OUTPUT);
  pinMode(moistureAnalogPin, INPUT);
  pinMode(battery3VAnalogPin, INPUT);
  pinMode(battery9VAnalogPin, INPUT);
  pinMode(temperatureAnalogPin, INPUT);
  pinMode(bucketEmptyPin, INPUT);
  digitalWrite(bucketEmptyPin, HIGH);  
  //Set motor control pin to output
  DDRB |= (1 << DDB7);
  
//  pinMode(nrf24PowerPin, OUTPUT);
//  digitalWrite(nrf24PowerPin, HIGH);
  setupRadio();
  setup_watchdog(9);
  delay(100);
}

// watchdog interrupt
ISR(WDT_vect) {
  watchdog_counter++;
}

void loop(void)
{
  if(watchdog_counter >= action.sleep/8) {
    watchdog_counter = 0;
    readSensors();
    radio.stopListening();
    // Send to hub
    send_time = millis();
    printf("sending %d bytes: counter=%d, tmp=%d, moist1=%d, moist2=%d, moist=%d, empty=%d\n", sizeof(sensors), sensors.counter, sensors.tmp36, sensors.moisture1, sensors.moisture2, sensors.moisture, sensors.bucketEmpty);
    if ( radio.write( &sensors, sizeof(sensors)) ) {
      Serial.println("Send successful");
    }
    else {
      Serial.println("Send failed");
    }
    
    Serial.println("wait");
    radio.startListening();
    delay(20);
    boolean gotPayload = waitForPayload();
    
//  digitalWrite(nrf24PowerPin, LOW);
    
    if(gotPayload && action.motor > 0) {
      Serial.print("running motor for "), Serial.print(action.motor); Serial.println("seconds.");
      pump(action.motor * 1000);
    }
    loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
    //delay(100);
  }
  sleep();
//  digitalWrite(nrf24PowerPin, HIGH);
//  delay(100);
//  setupRadio();

}

void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  Serial.println(ww);


  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);

  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS);

}

boolean waitForPayload() {
  boolean timeout = false;

  while(millis() - send_time < waitTime) {
    while ( radio.available() && !timeout ) {
      Serial.print("waited: "); Serial.println(millis() - send_time);
      radio.read(&action, sizeof(action));
      
      // Check for timeout and exit the while loop
      if ( millis() - send_time > radio.getMaxTimeout() ) {
        Serial.println("Timeout!!!");
        return false;
      }
      return true;
    }
  }
  return false;
}

void sleep(void) {
  // disable ADC
  byte old_ADCSRA = ADCSRA;
  ADCSRA = 0;

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
 
  radio.powerDown();
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();
  ADCSRA = old_ADCSRA;
  // radio.powerUp();
}


void readSensors(void) {
  sensors.counter = counter++;
  sensors.tmp36 = toMilliVolt(avgAnalogRead(temperatureAnalogPin), 1);
  sensors.battery3V=toMilliVolt(avgAnalogRead(battery3VAnalogPin), 1430.0/430.0);
  sensors.battery9V=toMilliVolt(avgAnalogRead(battery9VAnalogPin), 1120.0/120.0);
  readMoisture();
  sensors.bucketEmpty = !digitalRead(bucketEmptyPin);
}

int toMilliVolt(int value, int factor) {
  return round(value/1023.0 * 1089 * factor); //1.089 is calculated internal reference voltage
}

int avgAnalogRead(int pin) {
  analogRead(pin);
  int sum = 0;
  for(int i = 0; i<5; i++) {
    sum += analogRead(pin);
  }
  return sum/5;
}

void pump(int time) {
  PORTB |= (1 << PORTB7);
//  Serial.println("High");
//  digitalWrite(motorPin, HIGH);
  delay(time);
  PORTB &= ~(1 << PORTB7);
//  digitalWrite(motorPin, LOW);
}

void readMoisture(void){
  setSensorPolarity(true);
  delay(flipTimer);
  sensors.moisture1 = avgAnalogRead(moistureAnalogPin); 
  setSensorPolarity(false);
  delay(flipTimer);
  // invert the reading
  sensors.moisture2 = 1023 - avgAnalogRead(moistureAnalogPin);
  sensors.moisture = (sensors.moisture1 + sensors.moisture2) / 2;
  //
  digitalWrite(voltageFlipPin1, LOW);
  digitalWrite(voltageFlipPin2, LOW);
}

void setSensorPolarity(boolean flip){
  if(flip){
    digitalWrite(voltageFlipPin1, HIGH);
    digitalWrite(voltageFlipPin2, LOW);
  }else{
    digitalWrite(voltageFlipPin1, LOW);
    digitalWrite(voltageFlipPin2, HIGH);
  }
}
void setupRadio(void) {
  radio.begin();

  // Enable this seems to work better
  radio.enableDynamicPayloads();
  
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(76);
  radio.setRetries(15,15);
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  
  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
}


