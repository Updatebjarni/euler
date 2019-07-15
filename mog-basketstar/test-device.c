#define BAUD_TOL 3
#define F_CPU 16000000
#define BAUD 115200

#define F_SCL 100000UL
#define TWI_PRESCALE 1
#define TWBR_VAL ((((F_CPU / F_SCL) / TWI_PRESCALE) - 16 ) / 2)

#include<stdio.h>
#include<string.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<util/setbaud.h>
#include<avr/pgmspace.h>


ISR(TWI_vect){

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
