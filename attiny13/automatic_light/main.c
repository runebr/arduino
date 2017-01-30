#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>

#define LED PB0
#define LDR_POWER PB1

volatile uint8_t counter1 = 0;
volatile uint8_t counter2 = 0;

uint8_t adc_read (void) {
  ADCSRA |= (1 << ADSC);         // start ADC measurement
  while (ADCSRA & (1 << ADSC) ); // wait till conversion complete 
  return ADCH;
}

ISR(PCINT0_vect) {

  PORTB |= _BV(LDR_POWER);
//  ADCSRA |= (1 << ADEN);    // Enable ADC 

  if(bit_is_set(PINB, PB3)  && adc_read() < 20) {
    PORTB |= _BV(LED);
    GIMSK &= ~(_BV(PCIE));
    set_sleep_mode(SLEEP_MODE_IDLE);
    // enable timer overflow interrupt
    TIMSK0 |=1<<TOIE0;
  }
  PORTB &= ~(_BV(LDR_POWER));  
//  ADCSRA &= ~(_BV(ADEN));
 /* else { */
 /*   PORTB &= ~(_BV(LED)); */
 /* } */
}

ISR(TIM0_OVF_vect) {
  if (++counter1 > 200) {   // a timer overflow occurs 4.6 times per second */
    counter1 = 0;
    if (++counter2 > 20) {   // a timer overflow occurs 4.6 times per second */
      PORTB &= ~(_BV(LED));  
      TIMSK0 &= ~(_BV(TOIE0));
      GIMSK |= 1 << PCIE;
      counter2 = 0;
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      PORTB = 0x00;
    }
  }
}

void goToSleep(void)
{
  byte adcsra, mcucr1, mcucr2;

//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
//  MCUCR &= ~(_BV(ISC01) | _BV(ISC00));      //INT0 on low level
//    GIMSK |= _BV(INT0);                       //enable INT0
  adcsra = ADCSRA;                          //save ADCSRA
  ADCSRA &= ~_BV(ADEN);                     //disable ADC
  cli();                                    //stop interrupts to ensure the BOD timed sequence executes as required
  mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  mcucr2 = mcucr1 & ~_BV(BODSE);            //if the MCU does not have BOD disable capability,
  MCUCR = mcucr1;                           //  this code has no effect
  MCUCR = mcucr2;
  sei();                                    //ensure interrupts enabled so we can wake up again
  sleep_cpu();                              //go to sleep
  sleep_disable();                          //wake up here
  ADCSRA = adcsra;                          //restore ADCSRA
}


void initADC()
{
  /* this function initialises the ADC 

        ADC Prescaler Notes:
	--------------------

	   ADC Prescaler needs to be set so that the ADC input frequency is between 50 - 200kHz.
  
           For more information, see table 17.5 "ADC Prescaler Selections" in 
           chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
          (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

           Valid prescaler values for various clock speeds
	
	     Clock   Available prescaler values
           ---------------------------------------
             1 MHz   8 (125kHz), 16 (62.5kHz)
             4 MHz   32 (125kHz), 64 (62.5kHz)
             8 MHz   64 (125kHz), 128 (62.5kHz)
            16 MHz   128 (125kHz)

           Below example set prescaler to 128 for mcu running at 8MHz
           (check the datasheet for the proper bit values to set the prescaler)
  */

  // 8-bit resolution
  // set ADLAR to 1 to enable the Left-shift result (only bits ADC9..ADC2 are available)
  // then, only reading ADCH is sufficient for 8-bit results (256 values)

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            (0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 64, bit 2 
            (0 << ADPS1) |     // set prescaler to 64, bit 1 
            (0 << ADPS0);      // set prescaler to 64, bit 0  
}

int main(void) {
// Set up Port B pin 0 mode to output
  DDRB |= 1<<DDB0;
  DDRB |= 1<<DDB1;
  DDRB |= 0<<DDB3;
  PORTB = 0x00;

  initADC();
  //ADCSRA |= (1 << ADEN);    // Enable ADC 

  // Set up pin change interrupt on pin PB3
  PCMSK |= 1 << PCINT3;
  // Enable pin change interrupt
  GIMSK |= 1 << PCIE;
  
  TCCR0B |= (1<<CS02) | (1<<CS00);    

  sei(); // Enable global interrupts 
  
  // Use the Power Down sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  while(1) {
    goToSleep();
  }
  while(1) {

    //sleep_mode();   // go to sleep and wait for interrupt...
    cli();
    //PRR |= (1<<PRTIM0) | (1<<PRADC);
    sleep_enable();
    sleep_bod_disable();
    /* BODCR |= (1<<BODS) | (1<<BODSE); */
    /* BODCR |= (1<<BODS); */
    /* BODCR &= ~(_BV(BODSE)); */
    sei();
    sleep_cpu();
    sleep_disable();
    /* if(bit_is_set(PINB, PB3)) { */
    /*   //Disable pin change interrupt */
    /* } else { */
    /* } */
  }
}
