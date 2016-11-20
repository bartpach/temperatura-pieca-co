#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "1Wire.h"

//Definicje port�w w pliku 1Wire.h


//---------------------------------------------
//	waitus() Dla zegara 4MHz, funkcja odmierza
//	tyle mikrosekund, co podany parametr tau
//---------------------------------------------




//---------------------------------------------
//	_1WireInit Funkcja inicjalizuj�ca 1Wire
//	funkcja zwraca:		0-nic nie wykryto
//						1-wykryto urz�dzenie
//						2-wykryto zwarcie
//---------------------------------------------
uint8_t _1WireInit( void )
{
	uint8_t InitVal=0;
	
	_1W_SET_0();	//stan 0 na linii
	_delay_us(500);	//op�nienie 500us
	_1W_SET_1();	//stan 1 na linii
	_delay_us(65);
	if(!(_1W_WE))	//r�wnoznaczne z if(!(PINB & 0x01)), patrz makro w 1Wire.h
		InitVal++;
	_delay_us(220);
	_delay_us(236);	//czekaj 416us do ko�ca inicjalizacji
	if(!(_1W_WE))	//je�li nadal PinB.0 = 0, zwarcie
		InitVal++;
	return InitVal;	//zwraca: 0-nic, 1-pastylka, 2-zwarcie
}

//---------------------------------------------
//	_1WireWriteSlot Slot zapisu bitu
//---------------------------------------------
void _1WireWriteSlot(uint8_t bit)
{
	if(bit)		//Write 1
	{
		_1W_SET_0();
		_delay_us(11);
		_1W_SET_1();
		_delay_us(60);
	}
	else		//Write 0
	{
		_1W_SET_0();
		_delay_us(70);
		_1W_SET_1();
	//	waitus(3);
	}
}

//---------------------------------------------
//	_1WireReadSlot Slot odczytu bitu
//---------------------------------------------
uint8_t _1WireReadSlot(void)
{
	uint8_t _bit=0;
	
	_1W_SET_0();	//ustaw 0
	_delay_us(1);
	_1W_SET_1();	//zwolnij magistral�, stan H z rezystora PullUp
	_delay_us(14);
	_bit = (_1W_WE);		//zwr�� warto�� PinB.0, 1 lub 0
	_delay_us(60);
	return _bit;
}

//---------------------------------------------
//	_1WireWriteByte Funkcja wysy�aj�ca ca�y
//	bajt danych na magistral�
//---------------------------------------------
void _1WireWriteByte(uint8_t dana)
{
	uint8_t i;
	for(i=0; i<8; i++)
	{
		_1WireWriteSlot( dana & 0x01 );		//wy�lij 1 lub 0
		dana >>= 1;
	}
}

//---------------------------------------------
//	_1WireReadByte Funkcja zwracaj�ca odczytany
//	bajt danych z magistrali
//---------------------------------------------
uint8_t _1WireReadByte(void)
{
	uint8_t i;
	uint8_t data=0;
	
	for(i=0x01; i!=0; i<<=1)	//za ka�dym obiegiem, przesu� jedynk� w lewo odpowiada
	{							//pozycjom zczytanej warto�ci (czytamy od LSB do MSB)
		if( _1WireReadSlot() )		//je�li _1WireReadSlot() odczyta� 1
			data |= i;			//umie�� 1 na odpowiedniej pozycji
	}
	return data;
}

//---------------------------------------------
//	_1WCalcCRC Wyliczanie CRC danych, funkcja
//	zapo�yczona z ksi��ki "Mikrokontrolery AVR
//	w praktyce" J. Doli�ski
//---------------------------------------------
void _1WCalcCRC(uint8_t bajt,uint8_t *CRC)      //procedura wyliczania CRC
{                  //wielomian generuj�cy jest r�wny: x^8 + x^5 + x^4 + 1
 uint8_t zp1,zp2,i;    //zmienne pomocnicze

 zp1=bajt;
 for(i=0;i<8;i++)
 {
  bajt^=*CRC;           //wska�nik *CRC wyznacza aktualnie wyliczony CRC
  zp2=bajt&0x01;        //wydzielenie bitu do oblicze�
  bajt=*CRC;
  if(zp2)
  {
   bajt^=0x18;
  }
  bajt=((uint8_t)(bajt)>>1)+0x80*zp2; //konwersja uch jest potrzebna do
                                            //prawid�owego wykonania przesuni�cia 
  *CRC=bajt;
  zp1=(bajt=zp1>>1);    
 }
}
//---------------------------------------------
//	_1WCheckCRC funkcja liczy CRC, argumenty:
//	wska�nik do tablicy z odczytanymi danymi
//	oraz wielko�� tablicy, je�li CRC si� zgadza
//	funkcja zwr�ci 0x00
//---------------------------------------------
uint8_t _1WCheckCRC( uint8_t *ptr, uint8_t size )
{
	uint8_t CRC=0;
	
	for(uint8_t i=0; i<size; i++)
	{
		_1WCalcCRC(ptr[i],&CRC);
	}
	return CRC;
}
