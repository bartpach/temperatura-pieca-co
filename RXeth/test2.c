/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher
 * Copyright: GPL V2
 *
 * Tuxgraphics AVR webserver/ethernet board
 *  
 * http://tuxgraphics.org/electronics/
 * Chip type           : Atmega88/168/328 with ENC28J60
 *********************************************/
#include <avr/io.h>
#include <string.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "timeout.h"
#include "avr_compat.h"
#include "net.h"
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>
#include <lcd/HD44780.h>
#include <lcd/HD44780.c>

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24};
// how did I get the mac addr? Translate the first 3 numbers into ascii is: TUX
static uint8_t myip[4] = {192,168,1,24};
// listen port for www
#define MYWWWPORT 81
//// listen port for udp
#define MYUDPPORT 1200
//

#define BUFFER_SIZE 450
static uint8_t buf[BUFFER_SIZE+1];
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

int main(void){
        uint16_t plen;
        uint16_t dat_p;
        uint8_t i=0;
        uint8_t payloadlen=0;
        
        // set the clock speed to 8MHz
        // set the clock prescaler. First write CLKPCE to enable setting of clock the
        // next four instructions.
        CLKPR=(1<<CLKPCE);
        CLKPR=0; // 8 MHZ
        _delay_loop_1(50); // 12ms
        
        // LED
        /* enable PB1, LED as output */
        DDRB|= (1<<DDB1);

        /* set output to Vcc, LED off */
        PORTB|= (1<<PORTB1);

        /*initialize enc28j60*/
        enc28j60Init(mymac);
        enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
        _delay_loop_1(50); // 12ms
        
        /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
        // LEDB=yellow LEDA=green
        //
        // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
        // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
        enc28j60PhyWrite(PHLCON,0x476);
        _delay_loop_1(50); // 12ms
        
        /* set output to GND, red LED on */
        PORTB &= ~(1<<PORTB1);
        i=1;
        
        //init the ethernet/ip layer:
        init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);


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


        while(1){




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

                // get the next new packet:
                plen = enc28j60PacketReceive(BUFFER_SIZE, buf);

                /*plen will ne unequal to zero if there is a valid 
                 * packet (without crc error) */
                if(plen==0){
                        continue;
                }
                // arp is broadcast if unknown but a host may also
                // verify the mac address by sending it to 
                // a unicast address.
                if(eth_type_is_arp_and_my_ip(buf,plen)){
                        make_arp_answer_from_request(buf);
                        continue;
                }
                // check if ip packets (icmp or udp) are for us:
                if(eth_type_is_ip_and_my_ip(buf,plen)==0){
                        continue;
                }
                
                if (i){
                        /* set output to Vcc, LED off */
                        PORTB|= (1<<PORTB1);
                        i=0;
                }else{
                        /* set output to GND, LED on */
                        PORTB &= ~(1<<PORTB1);
                        i=1;
                }

                        
                if(buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){
                        // a ping packet, let's send pong
                        make_echo_reply_from_request(buf,plen);
                        continue;
                }
                // tcp port www start, compare only the lower byte
                if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P]==MYWWWPORT){
                        if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V){
                                make_tcp_synack_from_syn(buf);
                                // make_tcp_synack_from_syn does already send the syn,ack
                                continue;
                        }
                        if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V){
                                init_len_info(buf); // init some data structures
                                // we can possibly have no data, just ack:
                                dat_p=get_tcp_data_pointer();
                                if (dat_p==0){
                                        if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V){
                                                // finack, answer with ack
                                                make_tcp_ack_from_any(buf);
                                        }
                                        // just an ack with no data, wait for next packet
                                        continue;
                                }
                                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                                        // head, post and other methods:
                                        //
                                        // for possible status codes see:
                                        // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                                        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
                                }else{
                                        // the "get" method

                                        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<p>--Temperatura piecaCO--</p>"));
                                        plen=fill_tcp_data_p(buf,plen,PSTR("TEMP: "));


                                        if (D==0)
                                       plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 0</font>"));
                                        else  if (D==1)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 1</font>"));

                                       else if (D==2)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 2</font>"));
                                       else if (D==3)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 3</font>"));
                                       else if (D==4)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 4</font>"));
                                       else  if (D==5)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 5</font>"));
                                       else  if (D==6)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 6</font>"));
                                       else if (D==7)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 7</font>"));
                                       else   if (D==8)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 8</font>"));
                                       else   if (D==9)
                                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\"> 9</font>"));


                                       if (J==0)
                                                                 plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">0</font>"));

                                       else  if (J==1)
                                                         	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">1</font>"));

                                       else   if (J==2)
                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">2</font>"));
                                       else    if (J==3)
                                                         	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">3</font>"));
                                       else    if (J==4)
                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">4</font>"));
                                       else     if (J==5)
                                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">5</font>"));
                                       else     if (J==6)
                                                         	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">6</font>"));
                                       else      if (J==7)
                                                                 plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">7</font>"));
                                       else   if (J==8)
                                                                 plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">8</font>"));
                                       else  if (J==9)
                                    	   plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">9</font>"));
                                       	if (bufor[3]=='0')
                                            plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.0</font>"));

                                           if (bufor[3]=='1')
                                             plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.1</font>"));

                                           if (bufor[3]=='2')
                                           	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.2</font>"));
                                            if (bufor[3]=='3')
                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.3</font>"));
                                            if (bufor[3]=='4')
                                                 plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.4</font>"));
                                            if (bufor[3]=='5')
                                           	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.5</font>"));
                                            if (bufor[3]=='6')
                                          	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.6</font>"));
                                             if (bufor[3]=='7')
                                           	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.7</font>"));
                                             if (bufor[3]=='8')
                                           	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.8</font>"));
                                             if (bufor[3]=='9')
                                           	     plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"#F00000\">.9</font>"));




                                }

                                make_tcp_ack_from_any(buf); // send ack for http get
                                make_tcp_ack_with_data(buf,plen); // send data
                                continue;
                        }

                }
                // udp interface:
                if (buf[IP_PROTO_P]==IP_PROTO_UDP_V){
                        payloadlen=buf[UDP_LEN_L_P]-UDP_HEADER_LEN;
                        // the received command has to start with t and be 4 char long
                        // e.g "test\0"
                        if (buf[UDP_DATA_P]=='t' && payloadlen==5){
                                make_udp_reply_from_request(buf,"hello",6,MYUDPPORT);
                        }
                }
        }
        return (0);
}
