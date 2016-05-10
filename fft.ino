#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

//UNO clock Frequency = 16 MHz
//It takes 13 clock cycles to finish analog reading
int refFrequency = 0;
int currentFrequency = 0;
int previousFrequency = 0;
int errorFrequency = 0;
int refState = 0;
int currentState = 0;
int tunningState = 0;
boolean inRange = false;
float P = 0;
float I = 0.0;
float D = 0.0;
float drive = 0;
int pwmDrive = 0;
const int refInput = 8;  //reference pin
const int currentInput = 9;  //final pin
const int ForMotorControl = 10;  //pump control pin PWM
const int BacMotorControl = 11;  //pump control pin PWM
const int tunning = 12;  //tuning control pin
int bin = 0;
int frequency = 0;
int resolution = 9615 / 256;
int maxLogOut = 0;


void setup() {
  Serial.begin(9600); // use the serial port
  pinMode(refInput, INPUT);
  pinMode(currentInput, INPUT);
  pinMode(tunning, INPUT);
  pinMode(ForMotorControl, OUTPUT);
  pinMode(BacMotorControl, OUTPUT);
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //set adc prescaler=128
  ADMUX = 0x40; // use adc0
  ADCSRA |= (1<<ADATE);  //enable auto trigger
  ADCSRA |= (1<<ADEN);  //enable ADC
  ADCSRA |= (1<<ADSC);  //start ADC measurements
}

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    //sampling
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA |=(1<<ADSC); //start ADC measurements
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }//end of for
    
    //FFT
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    maxLogOut = 0;
    for(int i = 1; i < 128; i++){
      if(fft_log_out[i] > maxLogOut && fft_log_out[i] > 150){
          maxLogOut = fft_log_out[i];
          bin = i;
      }//end of if
    }//end of for
    frequency = bin * resolution;
    if(frequency > 1500 && frequency < 2700){
      inRange = true;
    } else {
      inRange = false;
    }
    
    //if digital pin 8 is high, set the reference frequency
    //if digital pin 9 is high, set the current frequency
    //if digital pin 12 is high, set tunning state = high
    refState = digitalRead(refInput);
    currentState = digitalRead(currentInput);
    tunningState = digitalRead(tunning);
    if(refState == HIGH && inRange == true){
      refFrequency = frequency;
    } else if(currentState == HIGH && inRange == true){
      currentFrequency = frequency;
    } //end of else if
    
    //error tracking
    //highest frequency = (2405)2479 hz  lowest frequency = (1517) 1628 hz
    errorFrequency = refFrequency - currentFrequency;
    P = errorFrequency;  //proportional term
   // I = Integral * 0;  //intergral term
   // D = (previousFrequency - currentFrequency) * 0;  //derivative term
   // previousFrequency = currentFrequency;  
    drive = P * (255.0/888.0); //scaleFactor = 255 * 888;

    //start tuning if tunning state = high
    if(tunningState == HIGH){
    //motor control
      if(errorFrequency > 30){      //pump liquid out 
        //digitalWrite();  //pump in one direction
        digitalWrite(BacMotorControl, HIGH);
        analogWrite(ForMotorControl, 0);
      } else if (errorFrequency < -30){  //pump liquid in
        //digitalWrite();  //pump in another direction
        digitalWrite(BacMotorControl, LOW);
        pwmDrive = (int)abs(drive);
        if(pwmDrive < 80){ pwmDrive = pwmDrive + 80;}
        analogWrite(ForMotorControl, pwmDrive);
      } else {  //stop pumping
        analogWrite(ForMotorControl, 0);  //stop the motor
        digitalWrite(BacMotorControl, LOW);
      }
    } else {
      analogWrite(ForMotorControl, 0);  //stop the motor
      digitalWrite(BacMotorControl, LOW);
    }
    
    //debug
    Serial.print("frequency: ");
    Serial.print(frequency);
    Serial.print(" | ");
    Serial.print("reference frequency: ");
    Serial.print(refFrequency);
    Serial.print(" | ");
    Serial.print("Current frequency: ");
    Serial.print(currentFrequency);
    Serial.print(" | ");
    Serial.print("error frequency: ");
    Serial.println(errorFrequency);
  }//end of while
}//end of loop