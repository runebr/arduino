int photoRPin = A0; 
int ledPin = 3;
int photoControlPin = 7;
int lightLevel;
boolean ledOn = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(photoControlPin, OUTPUT);
}

void loop() {
  digitalWrite(photoControlPin, HIGH);
  delay(1000);
  lightLevel = analogRead(photoRPin);
  Serial.println(lightLevel);
//  if(lightLevel < 10 && !ledOn) {
//    digitalWrite(ledPin, HIGH);
//    ledOn = true;
//  } else if(lightLevel > 30 && ledOn) {
//    digitalWrite(ledPin, LOW);
//    ledOn = false;
//  }
  digitalWrite(photoControlPin, LOW);
  delay(1000);
}
