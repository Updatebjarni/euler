#define BAUD_TOL 3
#define F_CPU 16000000
#define BAUD 115200

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

<<<<<<< HEAD
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
=======
>>>>>>> f1b5456f034bc6ccd0b14d59a833ecaf5ae1d0f6

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
//  TWCR|=(1<<TWIE);
  TWCR=1<<TWEN;

  DDRB=1;
  PORTB=0;

  unsigned char count=0;
  while(1){
    for(long i=0; i<5000; ++i);
    PORTB^=1;

    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=(112<<1)|0;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=count++;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=count++;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWDR=count++;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));

    TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
//    while(!(TWCR&(1<<TWINT)));
    }
  }
