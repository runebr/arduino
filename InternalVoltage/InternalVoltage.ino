#include <avr/sleep.h>

void setup() {
  analogReference (INTERNAL); // Sets the internal 1.1 volt reference.
  analogRead (A0);            // Forces voltage reference to be turned on.

  // Put the ATmega in a low-power deep sleep.
  ADCSRA &= ~(1 << ADEN);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  sleep_enable();
  sleep_cpu();
}

void loop() {}
