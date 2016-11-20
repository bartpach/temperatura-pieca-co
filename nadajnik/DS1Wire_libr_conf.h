#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>
#include <lcd/HD44780.h>
#include <lcd/HD44780.c>

#define FOSC 16000000
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define PIN_1WIRE 0
#define PORT_1WIRE PINC
#define OUT_1WIRE_LOW PORT_1WIRE&=~(1<<PIN_1WIRE);
#define OUT_1WIRE_HIGH PORT_1WIRE|=(1<<PIN_1WIRE);
#define DIR_1WIRE_IN DDRC&=~(1<<PIN_1WIRE);
#define DIR_1WIRE_OUT DDRC|=(1<<PIN_1WIRE);

char cStringBuffer[8];

void USART_Transmit(unsigned char data);
void USART_Putstr( char* data);
void USART_Putint( int data);
unsigned char USART_Receive( void );
void USART_Init( unsigned int baud);
uint16_t pomiar (uint8_t kanal);
unsigned char uc1Wire_ResetPulse(void);
void v1Wire_SendBit(char cBit);
unsigned char uc1Wire_ReadBit(void);
void v1Wire_SendByte(char ucByteValue);
unsigned char uv1Wire_ReadByte(void);





int main(){
    LCD_Initalize();
	USART_Init(MYUBRR);
	uint8_t oscyl = OSCCAL;
	unsigned char ucReset;
	char cTemperatureH=0, cTemperatureL=0;
float fTemperature=0;
	while(1){
		float temp;
		float temp2;
		float i;

		ucReset=uc1Wire_ResetPulse();
		// if(ucReset==1)
		 {
			 v1Wire_SendByte(0xCC);
			 v1Wire_SendByte(0x44);
			 _delay_ms(750);
			 ucReset=uc1Wire_ResetPulse();
			 v1Wire_SendByte(0xCC);
			 v1Wire_SendByte(0xBE);
			 cTemperatureL=uv1Wire_ReadByte();
			 cTemperatureH=uv1Wire_ReadByte();
			 ucReset=uc1Wire_ResetPulse();

			 fTemperature=(float)(cTemperatureL+(cTemperatureH<<8))/1024;

			 dtostrf( fTemperature, 4, 1, cStringBuffer); //4 is mininum width, 3 is precision; float value is copied onto buff
				USART_Putstr( "Temperatura:" );
				_delay_ms(10);

				//USART_Putint(temp);
				USART_Putstr(cStringBuffer);
		 }
	/*	 else
		 {
			 USART_Putstr( "brak temperatury:" );
		 }*/
		LCD_Clear();

	}
}

void USART_Transmit(unsigned  char data){

	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;

}
void USART_Putstr( char* data){

	uint8_t i = 0;
	while(data[i] != 0x0){
		USART_Transmit(data[i]);
		i++;
	}
	_delay_ms(10);
	//USART_Transmit(0x0d);
	USART_Transmit(0x0a);
}
void USART_Putint( int data){
	char bufor[17];
	USART_Putstr( itoa(data, bufor, 10) );
}
void USART_Init( unsigned int ubrr){
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C =(3<<UCSZ00);

}

unsigned char USART_Receive( void )
{
/* Wait for data to be received */
while ( !(UCSR0A & (1<<RXC0)) );
/* Get and return received data from buffer */
return UDR0;
}

unsigned char uc1Wire_ResetPulse(void)
{
unsigned char ucPresenceImpulse;
OUT_1WIRE_LOW;
DIR_1WIRE_OUT;
_delay_us(500);
DIR_1WIRE_IN;
_delay_us(45);
if(bit_is_clear(PORT_1WIRE, PIN_1WIRE))
	ucPresenceImpulse=1;
else
	ucPresenceImpulse=0;
_delay_us(470);
return ucPresenceImpulse;
}

void v1Wire_SendBit(char cBit)
{
DIR_1WIRE_OUT;
_delay_us(5);

if(cBit==1)
	DIR_1WIRE_IN;
_delay_us(80);
DIR_1WIRE_IN;
}

unsigned char uc1Wire_ReadBit(void)
{
	unsigned char ucBit;

	DIR_1WIRE_OUT;
	_delay_us(2);
	DIR_1WIRE_IN;
	_delay_us(15);
	if(bit_is_set(PORT_1WIRE, PIN_1WIRE))
		ucBit=1;
	else
		ucBit=0;
	return(ucBit);
}

void v1Wire_SendByte(char ucByteValue)
{
unsigned char ucCounter;
unsigned char ucValueToSend;

for(ucCounter=0 ; ucCounter<8;ucCounter++)
{
	ucValueToSend = ucByteValue>>ucCounter;
	ucValueToSend &= 0x01;
	v1Wire_SendBit(ucValueToSend);
}
_delay_us(100);
}

unsigned char uv1Wire_ReadByte(void)
{
unsigned char ucCounter;
unsigned char ucReadByte=0;

for(ucCounter=0 ; ucCounter<8;ucCounter++)
{
	if (uc1Wire_ReadBit())
	ucReadByte|=0x01<<ucCounter;
	_delay_us(15);
}
return (ucReadByte);
}
