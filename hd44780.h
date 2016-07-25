#pragma once

#include "main.h" //wczytujê plik nag³ówkowy

/*
Plik hd44780.h
*/

/* RS */
#define SET_OUT_LCD_RS  DDRD  |=  _BV(PD0)
#define SET_LCD_RS      PORTD |=  _BV(PD0)
#define CLR_LCD_RS      PORTD &= ~_BV(PD0)

/* RW */
#define SET_OUT_LCD_RW  DDRD  |=  _BV(PD1)
#define SET_LCD_RW      PORTD |=  _BV(PD1)
#define CLR_LCD_RW      PORTD &= ~_BV(PD1)

/* E */
#define SET_OUT_LCD_E   DDRD  |=  _BV(PD2)
#define SET_LCD_E       PORTD |=  _BV(PD2)
#define CLR_LCD_E       PORTD &= ~_BV(PD2)

/* D4 */
#define SET_OUT_LCD_D4  DDRD  |=  _BV(PD3)
#define SET_IN_LCD_D4   DDRD  &= ~_BV(PD3)
#define SET_LCD_D4      PORTD |=  _BV(PD3)
#define CLR_LCD_D4      PORTD &= ~_BV(PD3)
#define IS_SET_LCD_D4   PIND  &   _BV(PD3)

/* D5 */
#define SET_OUT_LCD_D5  DDRD  |=  _BV(PD4)
#define SET_IN_LCD_D5   DDRD  &= ~_BV(PD4)
#define SET_LCD_D5      PORTD |=  _BV(PD4)
#define CLR_LCD_D5      PORTD &= ~_BV(PD4)
#define IS_SET_LCD_D5   PIND  &   _BV(PD4)

/* D6 */
#define SET_OUT_LCD_D6  DDRD  |=  _BV(PD5)
#define SET_IN_LCD_D6   DDRD  &= ~_BV(PD5)
#define SET_LCD_D6      PORTD |=  _BV(PD5)
#define CLR_LCD_D6      PORTD &= ~_BV(PD5)
#define IS_SET_LCD_D6   PIND  &   _BV(PD5)

/* D7 */
#define SET_OUT_LCD_D7  DDRD  |=  _BV(PD6)
#define SET_IN_LCD_D7   DDRD  &= ~_BV(PD6)
#define SET_LCD_D7      PORTD |=  _BV(PD6)
#define CLR_LCD_D7      PORTD &= ~_BV(PD6)
#define IS_SET_LCD_D7   PIND  &   _BV(PD6)


#define LCD_NOP asm volatile("nop\n\t""nop\n\t" "nop\n\t" "nop\n\t" ::);



#define LCDCOMMAND 0
#define LCDDATA    1

#define LCD_LOCATE(x,y)  WriteToLCD(0x80|((x)+((y)*0x40)), LCDCOMMAND)

#define LCD_CLEAR              WriteToLCD(0x01, LCDCOMMAND)
#define LCD_HOME               WriteToLCD(0x02, LCDCOMMAND)

/* IDS */

#define LCDINCREMENT           0x02
#define LCDDECREMENT           0x00
#define LCDDISPLAYSHIFT        0x01

#define LCD_ENTRY_MODE(IDS)    WriteToLCD(0x04|(IDS), LCDCOMMAND)

/* BCD */
#define LCDDISPLAY             0x04
#define LCDCURSOR              0x02
#define LCDBLINK               0x01

#define LCD_DISPLAY(DCB)       WriteToLCD(0x08|(DCB), LCDCOMMAND)

/* RL */
#define LCDLEFT                0x00
#define LCDRIGHT               0x04

#define LCD_SHIFT_DISPLAY(RL)  WriteToLCD(0x18|(RL), LCDCOMMAND)
#define LCD_SHIFT_CURSOR(RL)   WriteToLCD(0x10|(RL), LCDCOMMAND)

#define LCD_CGRAM_ADDRESS(A)   WriteToLCD(0x40|((A)&0x3f), LCDCOMMAND)
#define LCD_DDRAM_ADDRESS(A)   WriteToLCD(0x80|((A)&0x7f), LCDCOMMAND)

#define LCD_WRITE_DATA(D)      WriteToLCD((D),LCDDATA)


void lcd_init(void);
void WriteToLCD(unsigned char v, unsigned char rs);
unsigned char ReadAddressLCD(void);
void lcd_puts(char *str);
void lcd_puts(const char *str);
void lcd_puts(int a);
