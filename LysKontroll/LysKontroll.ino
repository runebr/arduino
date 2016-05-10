#include <OneWire.h>
#include <DallasTemperature.h>
#include <NexaCtrl.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include "printf.h"

#define ONE_WIRE_BUS 2
#define TX_PIN 6
#define RX_PIN 8
#define sign(x) ( (x>=0) ? +1 : -1 )

enum {LIGHTON, LIGHTOFF, DIM} lightstate = LIGHTOFF;
enum {WAIT_FOR_HIGH_SOUND, ZERO_CROSSINGS, CLAP_DETECTED, SINGLE_CLAP_DETECTED, DOUBLE_CLAP_DETECTED, DEBOUNCE, NONE} detectState = WAIT_FOR_HIGH_SOUND;
enum {TEMP_IDLE, TEMP_REQUEST, TEMP_CONVERSION, TEMP_READY} tempState = TEMP_REQUEST;
enum {WAIT, READ, WRITE} commState = WAIT;

const static unsigned int RESOLUTION = 12;

const static unsigned long controller_id = 10516222;
const static unsigned int ampThreshold = 50;
const static unsigned int ampHigh = 200;
const static unsigned long CLAP_TIME = 100;
const static unsigned long DEBOUNCE_TIME = 100;
const static unsigned long SECOND_CLAP_TIMEOUT = 500;
const static unsigned long TEMP_TIME = 1000 * 60L;

typedef struct {
  uint16_t counter;
  uint16_t temperature;
} data_t;
typedef struct {
  boolean light;
  uint8_t dim;
} command_t;


data_t data;
command_t command;

RF24 radio(9, 10);
byte addresses[][6] = {"1Node", "2Node"};


// Dallas temperature stuff
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
unsigned long lastTempRequest = 0;
int  tempDelayInMillis = 750 / (1 << (12 - RESOLUTION));
float temperature = 0.0;
int  idle = 0;
unsigned int device = 0;

NexaCtrl nexaCtrl(TX_PIN, RX_PIN);

int zeroCrossings = 0;
int highAmp = 0;
int maxAmp = 0;
int prev_amp = 0;
int claps = 0;
unsigned long zeroCrossingsStarted;
unsigned long debounceStarted;
unsigned long clapStarted;
unsigned long messageCounter = 0;

void setup() {
  Serial.begin(115200);
  printf_begin();

  pinMode(13, OUTPUT);
  pinMode(A0, INPUT);

  Serial.println("Set up radio");
  radio.begin();
  // enable dynamic payloads
  radio.enableDynamicPayloads();

  // optionally, increase the delay between retries & # of retries
  /* radio.setRetries(5,15); */

  //  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  Serial.print("Dallas Temperature Control Library Version: ");
  Serial.println(DALLASTEMPLIBVERSION);
  Serial.println("\n");

  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, RESOLUTION);

  sensors.setWaitForConversion(false);
  attachInterrupt(1, check_radio, LOW);
}

void loop() {
  unsigned long currentMillis = millis();
  int amp = analogRead(0) - 500;
  /* int amp = analogRead(0) - 495; */


  switch (commState) {
    case WAIT:
      break;
    case READ:
      Serial.println("READ");
      while (radio.available()) {
        radio.read(&command, sizeof(command));
        if (command.light) {
          Serial.println("Turning lights on");
          nexaCtrl.DeviceOn(controller_id, device);
          Serial.print("Dimlevel: "); Serial.println(command.dim);
          if (command.dim >= 0 && command.dim <= 15) {
            nexaCtrl.DeviceDim(controller_id, device, command.dim);
          }
        } else {
          Serial.println("Turning lights off");
          nexaCtrl.DeviceOff(controller_id, device);
        }
      }
      commState = WAIT;
      break;
    case WRITE:
      commState = WAIT;
      radio.stopListening();
      // Take the time, and send it.  This will block until complete
      Serial.print("Now sending length ");
      Serial.println(sizeof(data));
      radio.startWrite( &data, sizeof(data), 0);
      /* radio.write( &data, sizeof(data)); */
      // Now, continue listening
      /* radio.startListening(); */
      break;
  }


  switch (tempState) {
    case TEMP_IDLE :
      if (currentMillis - lastTempRequest > TEMP_TIME) {
        tempState = TEMP_REQUEST;
      }
      break;
    case TEMP_REQUEST :
      sensors.requestTemperatures();
      lastTempRequest = currentMillis;
      tempState = TEMP_CONVERSION;
      break;
    case TEMP_CONVERSION :
      if (currentMillis - lastTempRequest >= tempDelayInMillis) {
        Serial.print(" Temperature: ");
        temperature = sensors.getTempCByIndex(0);
        Serial.println(temperature, RESOLUTION - 8);
        tempState = TEMP_READY;
      }
      break;
    case TEMP_READY :
      //TODO: Send temp
      data.temperature = round(temperature * 100);
      data.counter = messageCounter++;
      commState = WRITE;
      tempState = TEMP_IDLE;
      break;
  }
  switch (detectState) {
    case DEBOUNCE :
      if (currentMillis - debounceStarted > DEBOUNCE_TIME) {
        Serial.print("zeroCrossings: "); Serial.println(zeroCrossings);
        Serial.print("highAmp: "); Serial.println(highAmp);
        Serial.print("maxAmp: "); Serial.println(maxAmp);
        if (highAmp > 10 && zeroCrossings > 50) {
          claps++;
        }
        if (claps > 1) {
          detectState = DOUBLE_CLAP_DETECTED;
        } else {
          detectState = WAIT_FOR_HIGH_SOUND;
        }
        zeroCrossings = 0;
        highAmp = 0;
        maxAmp = 0;
        prev_amp = 0;
      }
      break;
    case WAIT_FOR_HIGH_SOUND :
      if (claps > 0 && currentMillis - clapStarted > SECOND_CLAP_TIMEOUT) {
        detectState = SINGLE_CLAP_DETECTED;
      }
      if (abs(amp) > ampHigh) {
        zeroCrossingsStarted = currentMillis;
        detectState = ZERO_CROSSINGS;
        Serial.print("Detected high sound: "); Serial.println(amp);
      }
      break;
    case ZERO_CROSSINGS :
      /* if(claps > 0 && currentMillis - clapStarted > SECOND_CLAP_TIMEOUT) { */
      /*   detectState = SINGLE_CLAP_DETECTED; */
      /* } */
      if ((currentMillis - zeroCrossingsStarted < CLAP_TIME)) {
        if (sign(amp) != sign(prev_amp)) {
          zeroCrossings++;
        }
        if (abs(amp) > ampThreshold) {
          highAmp++;
        }
        if (abs(amp) > maxAmp) {
          maxAmp = amp;
        }
        prev_amp = amp;
      } else if (currentMillis - zeroCrossingsStarted > CLAP_TIME) {
        debounceStarted = currentMillis;
        clapStarted = currentMillis;
        detectState = DEBOUNCE;
      }
      break;
    case CLAP_DETECTED :
      break;
    case SINGLE_CLAP_DETECTED :
      nexaCtrl.DeviceOn(controller_id, device);
      digitalWrite(13, HIGH);
      detectState = WAIT_FOR_HIGH_SOUND;
      claps = 0;
      break;
    case DOUBLE_CLAP_DETECTED :
      nexaCtrl.DeviceOff(controller_id, device);
      digitalWrite(13, LOW);
      detectState = WAIT_FOR_HIGH_SOUND;
      claps = 0;
      break;
  }
}


void check_radio(void) {

  bool tx, fail, rx;
  radio.whatHappened(tx, fail, rx);

  if ( tx || fail ) {
    radio.startListening();
    Serial.println(tx ? F(":OK") : F(":Fail"));
  }

  if ( rx || radio.available()) {
    printf("Read\n\r");
    commState = READ;
  }
}
