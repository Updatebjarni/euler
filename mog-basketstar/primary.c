#define F_CPU 8000000
#define BAUD 19200

#define F_SCL 100000UL
#define TWI_PRESCALE 1
#define TWBR_VAL ((((F_CPU / F_SCL) / TWI_PRESCALE) - 16 ) / 2)

#define TWIBITS ((1<<TWIE)|(1<<TWEN)|(1<<TWEA)|(1<<TWINT))

#include<stdio.h>
#include<string.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<util/setbaud.h>
#include<util/atomic.h>
#include<avr/pgmspace.h>

#define PRINTF(fmt, ...) printf_P(PSTR(fmt), ## __VA_ARGS__)

// Pin assignments:
// PB0 Mogward clock  PC0 Bus 0       PD0 RXD
// PB1 Data ready     PC1 Bus 1       PD1 TXD
// PB2 Busy           PC2 Bus 2       PD2 INT0 (AS)
// PB3 MOSI           PC3 Bus 3       PD3 INT1 (WS)
// PB4 MISO           PC4 SDA         PD4 Bus 4
// PB5 SCK            PC5 SCL         PD5 Bus 5
// PB6 Data OE        PC6 RESET       PD6 Bus 6
// PB7 Addr OE                        PD7 Bus 7

// This is a buffer of 3-byte packets on their way to the parallel output
volatile unsigned char mogward_buf[64][3], mogward_head, mogward_qlen;

// This one is addr/data pairs headed for the secondary on the UART port
volatile unsigned char secward_buf[64][2], secward_head, secward_qlen;

// And this one is the same but for data going to devices on the i2c bus
volatile unsigned char i2cward_buf[64][2], i2cward_head, i2cward_qlen;


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

enum{OUT, IN};
void busmode(char mode){
  if(mode==OUT){
    DDRC|=0x0F;
    DDRD|=0xF0;
    }
  else{
    DDRC&=~0x0F;
    DDRD&=~0xF0;
    }
  }

// INT0 is MOG's Address Strobe input, which latches data into the shift
// registers. We can now put new data on the shift register inputs for next AS.
ISR(INT0_vect){
  if(mogward_qlen){
    unsigned char tail=(mogward_head-mogward_qlen)&63;
    busmode(OUT);
    for(short i=0; i<3; ++i){
      unsigned char byte=mogward_buf[tail][i];
      PORTC=(PORTC&0xF0)|(byte&0x0F);
      PORTD=(PORTD&0x0F)|(byte&0xF0);
      PORTB^=1; PORTB^=1;  // Load mogward shift register
      }
    busmode(IN);
    --mogward_qlen;
    PORTB^=2; PORTB^=2;  // Set Data Ready flip-flop
    }
  }

// Drops packets silently if the buffer is full
void queue_mogward(unsigned char *packet){
  if(mogward_qlen==64)return;
  for(short i=0; i<3; ++i)
    mogward_buf[mogward_head][i]=packet[i];
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    mogward_head=(mogward_head+1)&63;
    ++mogward_qlen;
    }
  }

ISR(INT1_vect){
  unsigned char addr, data;
  PORTB&=0x7F;  // /OE low on the address byte buffer
  addr=(PORTC&0x0F)|(PORTD&0xF0);
  PORTB|=0x80;  // /OE high
  PORTB&=0xBF;  // /OE low on the data byte buffer
  data=(PORTC&0x0F)|(PORTD&0xF0);
  PORTB|=0x40;  // /OE high

  char bus=addr&32;
  volatile unsigned char (*buf)[2]=bus?secward_buf:i2cward_buf;
  volatile unsigned char *qlen=bus?&secward_qlen:&i2cward_qlen;
  volatile unsigned char *head=bus?&secward_head:&i2cward_head;
  if(*qlen<64){
    buf[*head][0]=addr&31;
    buf[*head][1]=data;
    *head=((*head)+1)&63;
    ++*qlen;
    }

  if(secward_qlen<64 && i2cward_qlen<64){
    PORTB^=4; PORTB^=4;  // Clear Busy flip-flop
    }
  }

ISR(TWI_vect){
  static unsigned char recv_buf[3];
  static short recv_byteno;

  switch(TWSR){
    case 0x60:  // We are being addressed
    case 0x68:  // We lost arbitration and are being addressed
      TWCR=TWIBITS;
      recv_byteno=0;
      return;
    case 0x80:  // We have received data and acked them
      recv_buf[recv_byteno++]=TWDR;
      TWCR=TWIBITS;
      if(recv_byteno<3)return;
      queue_mogward(recv_buf);
      break;
    case 0xA0:  // We have received a stop or repeated start
      TWCR=TWIBITS;
      break;
    case 0x00:  // Bus error
      TWCR=TWIBITS|(1<<TWSTO);
      break;
    }

  // *** check if i2cward data are waiting to be sent, and start sending them

//  TWCR=TWIBITS;
  }

int main(){
  DDRB=7+64+128;
  DDRC=0;
  DDRD=0;
  PORTB=0xC3;

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
  TWBR=TWBR_VAL;
  TWAR=(112<<1);
  TWCR=(1<<TWEN)|(1<<TWIE)|(1<<TWEA);

  EICRA=3;
  EIMSK=1;

  sei();
  while(1);
  }
