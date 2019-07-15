#define F_CPU 16000000
#define BAUD 19200

#define F_SCL 100000UL
#define TWI_PRESCALE 1
#define TWBR_VAL ((((F_CPU / F_SCL) / TWI_PRESCALE) - 16 ) / 2)

#define TWIBITS ((1<<TWIE)|(1<<TWEN)|(1<<TWEA)|(1<<TWINT))
#define ADCBITS ((1<<ADEN)|(1<<ADSC)|(1<<ADIE)|7)

#include<stdio.h>
#include<string.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<util/setbaud.h>
#include<avr/pgmspace.h>

volatile unsigned short sample;
volatile char i2c_idle=1;

ISR(TWI_vect){
  switch(TWSR){
    case 0x60:  // We are being addressed
    case 0x68:  // We lost arbitration and are being addressed
      TWCR=TWIBITS;
//      recv_byteno=0;
      return;
    case 0x80:  // We have received data and acked them
//      recv_buf[recv_byteno++]=TWDR;
      TWCR=TWIBITS;
//      if(recv_byteno<3)return;
//      queue_mogward(recv_buf);
      break;
    case 0xA0:  // We have received a stop or repeated start
      TWCR=TWIBITS;
      break;
    case 0x00:  // Bus error
      TWCR=TWIBITS|(1<<TWSTO);
      break;
    }

  }

ISR(TIMER0_COMPA_vect){  // This interrupt is triggered about once per 10ms.
  ADCSRA=ADCBITS;
  }

ISR(ADC_vect){
  sample=ADCL|(ADCH<<8)|0x8000;
  if(i2c_idle){
    TWCR=TWIBITS|(1<<TWSTA);
    i2c_idle=0;
    }
  }

int main(){

  // I2C
  TWBR=TWBR_VAL;
  TWAR=(1<<1)|1;
  TWCR=(1<<TWEN)|(1<<TWIE)|(1<<TWEA);

  // ADC
  ADMUX=64;  // Internal AVcc reference, sample ADC0 pin
  ADCSRA=ADCBITS;

  // Timer
  TCNT0=0;         // Initialise timer count.
  OCR0A=155;       // Timer counts to 155, taking about 10ms at 16MHz/1 clock.
  TCCR0A=2;        // "Clear timer on compare" mode
  TCCR0B=5;        // Prescaler=1024. This starts the timer.
  TIMSK0=2;        // Generate interrupt on compare match.

  DDRB=1;
  PORTB=0;

  sei();

  unsigned char count=0;
  while(1){
    unsigned short tosend;
    while(!(tosend=sample));
    
    tosend=(tosend*2)&0x7FF;

    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=(112<<1)|0;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=1;  // Our address
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=tosend>>8;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=tosend&255;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
//    while(!(TWCR&(1<<TWINT)));
    }
  }
