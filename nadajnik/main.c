#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>
#include <lcd/HD44780.h>
#include <lcd/HD44780.c>
#include "1Wire.h"

#define FOSC 16000000
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


void USART_Transmit(unsigned char data);
void USART_Putstr( char* data);
void USART_Putint( int data);
unsigned char USART_Receive( void );
void USART_Init( unsigned int baud);
//float DS18B20_temp(void);


uint16_t pomiar (uint8_t kanal){
	ADMUX|=0x00;
	//ADMUX|=(ADMUX & 0xF8)|kanal;
	ADCSRA|=(1<<ADSC);
	return ADCW;
}




int main(){
	USART_Init(MYUBRR);

	 ADMUX |= (1<<REFS0);
		    ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128


	while(1){

		char bufor[17];
	//	float a = DS18B20_temp();
		//a=(a*1.0904);
		float a=(5.0 * pomiar(0) * 100.0) / 1024;
	dtostrf(a, 4, 1, bufor);
	USART_Putstr(bufor);
	_delay_ms(1000);

	}
}




//44,9 48,9
//44,8 48,7
//44,7 48,6
//44,6 48,5
//44,5 48,4
// 44,4 48,2




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
/*float DS18B20_temp(void)
{
	uint8_t data1;
	//INICJALIZACJA
			uint8_t tempH, tempL;
			uint16_t temp;
			data1 = _1WireInit();
			if(data1==1)//1 - oznacza udan¹ inicjalizacjê
			{
				_1WireWriteByte(0xCC);
				_1WireWriteByte(0x44);
				//odczekaæ min 750ms
				for(int k=0; k<1000; k++)
				{
					_delay_us(250);
					_delay_us(250);
					_delay_us(250);
				}

				data1 = _1WireInit();
				_1WireWriteByte(0xCC);
				_1WireWriteByte(0xBE);


				tempL = _1WireReadByte();
				tempH = _1WireReadByte();
	temp = (((int16_t)tempH) << 8) | tempL;

				//konwersja temperatury

				if(tempH & 0x80)
					USART_Putstr("-");
				//return (float)temp *  0.06833;    //0.0625;
				// return (float)(temp >> 1) * 0.138;   //0.125;
				// return (float)(temp >> 2) * 0.273;
				// return (float)(temp >> 3) * 0.5;
			}
			else if(data1==0)	//0 - oznacza "nic nie wykryto"
				USART_Putstr("nic");
			else if(data1==2)	//2 - oznacza "zwarcie"
				USART_Putstr("zwarcie");

}*/

