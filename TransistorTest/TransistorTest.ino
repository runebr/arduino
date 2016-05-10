int motorPin = 4;
void setup(void) {
  Serial.begin(57600);

  //pinMode(motorPin, OUTPUT);
  DDRB |= (1 << DDB7);

}
void loop(void)
{
 // set PB7 high
  PORTB |= (1 << PORTB7);
  Serial.println("High");
  delay(30000);
  // set PB7 low
  PORTB &= ~(1 << PORTB7);
  Serial.println("Low");
  delay(30000);

//  digitalWrite(motorPin, HIGH);
//  Serial.println("High");
//  delay(30000);
//  digitalWrite(motorPin, LOW);
//  Serial.println("Low");
//  delay(30000);
}
