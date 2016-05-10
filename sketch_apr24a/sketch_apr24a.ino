#define sign(x) ( (x>=520) ? +1 : -1 )

const int analogPin = 0;

void setup() {
    Serial.begin(115200);
}
 
void loop() {
    int mn = 1024; // mn only decreases
    int mx = 0;    // mx only increases
    int prev_val = 0;
    int zc = 0;
    int highAmp = 0;
    
    // Perform 10000 reads. Update mn and mx for each one.
    for (int i = 0; i < 10000; ++i) {
        int val = analogRead(analogPin);
        if (sign(val) != sign(prev_val)) {
          zc++;
        }
        if (abs(val-500) > 50) {
          highAmp++;
        }
        mn = min(mn, val);
        mx = max(mx, val);
    }
 
    // Send min, max and delta over Serial
    Serial.print("z=");
    Serial.print(zc);
    Serial.print(" h=");
    Serial.print(highAmp);
    Serial.print(" m=");
    Serial.print(mn);
    Serial.print(" M=");
    Serial.print(mx);
    Serial.print(" D=");
    Serial.print(mx-mn);
    Serial.println();
}
