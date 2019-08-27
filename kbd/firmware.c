#define F_CPU 8000000
#define BAUD 9600

#define F_SCL 400000UL
#define Prescaler 1
#define TWBR_val (((F_CPU / F_SCL) - 16 ) / 2 / Prescaler)

#include<stdio.h>
#include<string.h>
#include<avr/io.h>
#include<util/delay.h>
#include<util/setbaud.h>
#include<avr/pgmspace.h>
#include <util/twi.h>

#define PRINTF(fmt, ...) printf_P(PSTR(fmt), ## __VA_ARGS__)

typedef unsigned char byte;

#define I2C_WRITE 0
#define I2C_READ  1

char i2c_begin(byte addr, byte dir){
  TWCR=0;
  TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT)));
        
  if((TWSR&0xF8) != TW_START)return 1;
        
  TWDR=(addr<<1)|dir;
  TWCR=(1<<TWINT)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT)));
        
  byte status=TW_STATUS&0xF8;
  return (status!=TW_MT_SLA_ACK) && (status!=TW_MR_SLA_ACK);
  }

byte i2c_write(byte data){
  TWDR=data;
  TWCR=(1<<TWINT)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT)));
        
  return (TWSR & 0xF8) != TW_MT_DATA_ACK;
  }

byte i2c_read_ack(){
  TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA); 
  while(!(TWCR & (1<<TWINT)));
  return TWDR;
  }

byte i2c_read_nak(){
  TWCR=(1<<TWINT)|(1<<TWEN);
  while(!(TWCR & (1<<TWINT)));
  return TWDR;
  }

void i2c_end(){
  TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
  }

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

  // ## I2C ##
  TWBR=TWBR_val;
  PORTC|=48;

  // ## Row pin pull-ups ##
  PORTC|=0x0F;
  PORTD|=0xF0;

  i2c_begin(0x20, I2C_WRITE);
  i2c_write(0x0A);  // IOCON (BANK=0)
  i2c_write(0x20);  // Turn off sequential writes and set BANK=0
  i2c_end();
  i2c_begin(0x20, I2C_WRITE);
  i2c_write(0x00);  // IODIRA (BANK=0)
  i2c_write(0xFF);  // Port A all inputs
  i2c_write(0xFF);  // Port B all inputs
  i2c_end();
  i2c_begin(0x20, I2C_WRITE);
  i2c_write(0x0C);  // GPPUA (BANK=0)
  i2c_write(0x00);  // Port A all pull-ups off
  i2c_write(0x00);  // Port B all pull-ups off
  i2c_end();
  i2c_begin(0x20, I2C_WRITE);
  i2c_write(0x12);  // GPIOA (BANK=0)
  i2c_write(0x00);  // Port A all zeros
  i2c_write(0x00);  // Port B all zeros
  i2c_end();
  i2c_begin(0x20, I2C_WRITE);
  i2c_write(0x0A);  // IOCON (BANK=0)
  i2c_write(0xA0);  // Turn off sequential writes and set BANK=1
  i2c_end();

  byte reg=0x00;  // IODIRA to begin with

  byte keys[16]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  byte i=0;
  while(1){
    i2c_begin(0x20, I2C_WRITE);
    i2c_write(reg);

    for(byte v=1; v; v<<=1, ++i){
      byte newkeys;
      i2c_write(~v);
      newkeys=(PINC&0x0F)|(PIND&0xF0);
      if(newkeys!=keys[i&15]){
        keys[i&15]=newkeys;
        printf("keys[%d]: %02X\n", i&15, keys[i&15]);
        }
      }

    i2c_write(0xFF);
    i2c_end();

    reg^=0x10;
    }

  }
