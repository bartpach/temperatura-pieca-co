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
#define LED (1<<PC5)
#define BUZZ (1<<PC4)

volatile char bufor[10];
volatile uint8_t _bufor=0, bufor_ready=0,_buf;

 volatile char i;
// function to initialize UART
 void USART_Init( unsigned int ubrr){
 	/*Set baud rate */
 	UBRR0H = (unsigned char)(ubrr>>8);
 	UBRR0L = (unsigned char)ubrr;
 	//Enable receiver and transmitter
 	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
 	/* Set frame format: 8data, 1stop bit */
 	UCSR0C =(3<<UCSZ00);
 	UCSR0B |=(1<<RXCIE0);

 }


// function to receive data
unsigned char uart_recieve (void)
{
    while(!(UCSR0A) & (1<<RXC0));                   // wait while data is being received
    return UDR0;                                   // return 8-bit data

}

ISR(USART_RX_vect)
{

	if(!bufor_ready){

		bufor[_bufor]= UDR0;
			if( bufor[_bufor] == '\n' ){
				_buf=_bufor;
				_bufor= 0;
		//		bufor_ready= 1;
			}else
				{_bufor++;}

		}

}

int main(void)
{
	LCD_Initalize();
	uint8_t buzz=0;

	USART_Init(MYUBRR);
   sei();
   DDRC |= LED;
   DDRC |= BUZZ;
   char start[]={'-','-','T','e','m','p',' ','p','i','e','c','a','C','O','-','-'};
   for(uint8_t b=0;b!=16;b++){
     	   LCD_WriteData(start[b]);
     	   _delay_ms(100);
   }
   _delay_ms(1000);
   LCD_Clear();
   buzz=0;
    while(1)
    {

    	int D = bufor[0] - 48;
    	int J = bufor[1] - 48;
    	int temp_int = 10*D + J;



    	 LCD_GoTo(0,0);
         LCD_WriteText("--Temp piecaCO--");
         LCD_GoTo(0,1);
         LCD_WriteText("***** ");
for(uint8_t a=0;a!=(_buf);a++)
             LCD_WriteData(bufor[a]);
		LCD_WriteText(" *****");





		if (temp_int == 37 && buzz==0 && bufor[3]=='0')
		{
				 PORTC^=BUZZ;
				 _delay_ms(100);
				 PORTC^=BUZZ;
				 _delay_ms(100);
				 PORTC^=BUZZ;
				 _delay_ms(100);
				 PORTC^=BUZZ;
		         buzz++;
		}
		else if(temp_int >= 38 && bufor[3]>='0' )   // jeœli jest wiêcej ni¿ 40,0 to zmieñ œwiat³o na inne i zezwól na sygnal
		{
			buzz=0;
			PORTC&=~LED;

		}

		else                 // w ka¿dej innej styucaji zapal niebiesk¹ i zezwól na sygnal
		{

					//buzz=0;
				PORTC|=LED;

		}

		 _delay_ms(500);



     }
}
