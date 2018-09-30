#include<avr/sleep.h>
#include<avr/interrupt.h>
#include<avr/io.h>

volatile char ac_present;

ISR(PCINT0_vect){  // This interrupt is triggered by the AC waveform
  ac_present=1;    // in the organ's back plane power supply.
  }

ISR(TIM0_COMPA_vect){  // This interrupt is triggered about once per 50ms.
  static unsigned char wait=0;
  char ac_state=ac_present;  // Has this been set since we cleared it 50ms ago?
  char pc_state=PINB&1;      // Checks the state of the +5V rail of the PC.
  ac_present=0;

  if(DDRB&(1<<3))   // If we are currently "pressing the power switch",
    DDRB&=~(1<<3);  // then we release it now, by tristating it.

  if(wait){         // This means we are waiting for a while after pressing
    --wait;         // the switch before pressing it again, to allow the PC
    return;         // time to wake up or go to sleep.
    }

  if(pc_state!=ac_state){  // If the power states of the organ and PC differ,
    DDRB|=(1<<3);          // then we "press the power button" of the PC.
    wait=pc_state?120:20;  // Wait briefly for wake, longer (~6s) for sleep.
    }

  }

int main(){
  DDRB&=~(1<<3);   // Pin PB3 is the power switch control output.
  PORTB&=~(1<<3);  // We set it to 0 but input; switch to output to "press".

  GIMSK=32;        // Pin change interrupt enable for AC detection.
  PCMSK=0x10;      // Interrupt on pin PCINT4 (pin 3, also PB4).

  TCNT0=0;         // Initialise timer count.
  OCR0A=60;        // Timer counts to 60, taking about 50ms at 9.6MHz/8 clock.
  TCCR0A=2;        // "Clear timer on compare" mode (restart at count of 60)
  TCCR0B=5;        // Prescaler=1024. This starts the timer.
  TIMSK0=4;        // Generate interrupt on compare match.

  sei();
  set_sleep_mode(SLEEP_MODE_IDLE);

  while(1)         // The rest is handled by the interrupts, so we just sleep.
    sleep_mode();

  }
