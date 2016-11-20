#ifndef IWIRE_H_INCLUDED
#define IWIRE_H_INCLUDED

#define _1WPORT	C
#define _1WPIN	C
#define _1WSLOT	(1<<0)
#define _1W_WE	{ (PINC & _1WSLOT); }


#define _1W_SET_0()	{ DDRC |= 0x01; }	//PC.0 jako wyjœcie,
										//PC.0 = 0, wiêc stan na linii wynosi 0
#define _1W_SET_1()	{ DDRC &= 0xFE;}	//PC.0 jako wejœcie,
										//stan 1 pochodzi z rezystora PullUp

uint8_t		_1WireInit( void );
void		_1WireWriteByte(uint8_t dana);
uint8_t		_1WireReadByte(void);
void 		_1WCalcCRC(uint8_t bajt,uint8_t *CRC);
uint8_t		_1WCheckCRC( uint8_t *ptr, uint8_t size );
//	poni¿sze dwie funkcje s¹ u¿ywane tylko
//	w pliku 1Wire.c nie potrzeba ich na zewn¹trz
//void		_1WireWriteSlot(uint8_t bit);
//uint8_t	_1WireReadSlot(void);

#endif
