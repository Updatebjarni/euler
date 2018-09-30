#define BAUD_TOL 3
#define F_CPU 8000000
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

#define PRINTF(fmt, ...) printf_P(PSTR(fmt), ## __VA_ARGS__)

int uart_putc(char c, FILE *stream){
  while(!(UCSR0A&(1<<UDRE0)));
  UDR0=c;
  return 0;
  }

int uart_getc(FILE *stream){
  int c;
  do{
    do{
      if(UCSR0A&((1<<FE0)|(1<<DOR0)|(1<<UPE0))){
        c=UDR0; UCSR0A&=~((1<<FE0)|(1<<DOR0)|(1<<UPE0));
        }
      }while(!(UCSR0A&(1<<RXC0)));
    c=UDR0;
    }while(c=='\r');
  return c;
  }

ISR(TWI_vect){

  }

int main(){
  // ## Serial port ##
  fdevopen(uart_putc, uart_getc);
  UBRR0H=UBRRH_VALUE;
  UBRR0L=UBRRL_VALUE;
#if USE_2X
  UCSR0A|=(1<<U2X0);
#else
  UCSR0A&=~(1<<U2X0);
#endif
  UCSR0C=3<<UCSZ00;
  UCSR0B=(1<<RXEN0)|(1<<TXEN0);

  // I2C
  TWBR=TWBR_VAL;
  TWAR=(1<<1)|1;
//  TWCR|=(1<<TWIE);

  DDRB=1;

  unsigned char count=0;
  while(1){
    for(long i=0; i<100000; ++i);
    PORTB^=1;
    TWCR=0;
    TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));
    TWDR=(112<<1)|0;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));
    TWDR=count++;
    TWCR=(1<<TWINT)|(1<<TWEN);
    while(!(TWCR&(1<<TWINT)));
    TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
//    while(!(TWCR&(1<<TWINT)));
    }
  }
