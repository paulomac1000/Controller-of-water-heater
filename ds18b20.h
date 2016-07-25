#pragma once

#include "main.h" //wczytujê plik nag³ówkowy

/*
Plik ds18b20.h

(xyz.isgreat.org)
*/

/* DS18B20 przy³¹czony do portu  PC5AVRa  */
#define SET_ONEWIRE_PORT     PORTC  |=  _BV(5)
#define CLR_ONEWIRE_PORT     PORTC  &= ~_BV(5)
#define IS_SET_ONEWIRE_PIN   PINC   &   _BV(5)
#define SET_OUT_ONEWIRE_DDR  DDRC   |=  _BV(5)
#define SET_IN_ONEWIRE_DDR   DDRC   &= ~_BV(5)

unsigned char ds18b20_ConvertT(void);
int ds18b20_Read(unsigned char[]);
void OneWireStrong(char);
unsigned char OneWireReset(void);
void OneWireWriteByte(unsigned char);
unsigned char OneWireReadByte(void);
