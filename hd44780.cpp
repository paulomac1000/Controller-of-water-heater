/*
  Plik hd44780.c

  Definicje kilku funkcji do obs³ugi alfanumerycznego
  wyœwietlacza LCD HD44780
*/

#include "hd44780.h"

/*--------------------------------------------------------*/
/* Zapis danej lub instrukcji */

void WriteToLCD (unsigned char v,unsigned char  rs)
{
    unsigned char bf;

    SET_OUT_LCD_D4;
    SET_OUT_LCD_D5;
    SET_OUT_LCD_D6;
    SET_OUT_LCD_D7;

    if(v&0x10) SET_LCD_D4; else CLR_LCD_D4;
    if(v&0x20) SET_LCD_D5; else CLR_LCD_D5;
    if(v&0x40) SET_LCD_D6; else CLR_LCD_D6;
    if(v&0x80) SET_LCD_D7; else CLR_LCD_D7;
 
    CLR_LCD_E;
    if(rs) SET_LCD_RS;else CLR_LCD_RS;
    CLR_LCD_RW;

    LCD_NOP;
    SET_LCD_E;
    LCD_NOP; 
    CLR_LCD_E;
    LCD_NOP;
 
    if(v&0x01) SET_LCD_D4; else CLR_LCD_D4;
    if(v&0x02) SET_LCD_D5; else CLR_LCD_D5;
    if(v&0x04) SET_LCD_D6; else CLR_LCD_D6;
    if(v&0x08) SET_LCD_D7; else CLR_LCD_D7;
 
    LCD_NOP;
    SET_LCD_E;
    LCD_NOP; 
    CLR_LCD_E;
    LCD_NOP;
 
    SET_IN_LCD_D4;
    SET_IN_LCD_D5;
    SET_IN_LCD_D6;
    SET_IN_LCD_D7;

    CLR_LCD_RS;
    SET_LCD_RW;
    SET_LCD_D7;


/* Przyda³by siê pe³ny odczyt */
    do
    {
        LCD_NOP;
        SET_LCD_E;
        LCD_NOP;
        bf = IS_SET_LCD_D7;
        CLR_LCD_E;
        LCD_NOP;
        SET_LCD_E;
        LCD_NOP;
        LCD_NOP;
        CLR_LCD_E;
        
    }while( bf );
}


/*--------------------------------------------------------*/
/* Funkcja odczytuje adres i flage zajetosci */

unsigned char ReadAddressLCD (void)
{
    unsigned char g = 0 ;

    CLR_LCD_RS;
    SET_LCD_RW; 

    SET_IN_LCD_D4;
    SET_IN_LCD_D5;
    SET_IN_LCD_D6;
    SET_IN_LCD_D7;

    LCD_NOP;
    SET_LCD_E;
    LCD_NOP;

    if(IS_SET_LCD_D4) g+=16;
    if(IS_SET_LCD_D4) g+=32;
    if(IS_SET_LCD_D4) g+=64;
    if(IS_SET_LCD_D4) g+=128;
 
    CLR_LCD_E;
    LCD_NOP;
    SET_LCD_E;  
    LCD_NOP;
  
    if(IS_SET_LCD_D4) g+=8;
    if(IS_SET_LCD_D4) g+=4;
    if(IS_SET_LCD_D4) g+=2;
    if(IS_SET_LCD_D4) g+=1;
  
    CLR_LCD_E; 

    return  g ;
}


/*---------------------------------------------------------*/
/* Inicjalizacja wyœwietlacza */

void lcd_init(void)
{
    _delay_ms(31);    
   
    SET_OUT_LCD_RS;
    SET_OUT_LCD_RW;
    SET_OUT_LCD_E;
    SET_OUT_LCD_D4;
    SET_OUT_LCD_D5;
    SET_OUT_LCD_D6;
    SET_OUT_LCD_D7;

    CLR_LCD_E;
    CLR_LCD_RS;
    CLR_LCD_RW;
    SET_LCD_D4;
    SET_LCD_D5;
    CLR_LCD_D6;
    CLR_LCD_D7;        
  
    LCD_NOP;
    SET_LCD_E;
    LCD_NOP; 
    CLR_LCD_E;
    LCD_NOP;
    _delay_ms(10);

    LCD_NOP;
    SET_LCD_E;
    LCD_NOP; 
    CLR_LCD_E;
    LCD_NOP;
    _delay_ms(2);

    LCD_NOP;
    SET_LCD_E;
    LCD_NOP; 
    CLR_LCD_E;
    LCD_NOP;
    _delay_ms(2);

    CLR_LCD_D4;
    LCD_NOP;
    SET_LCD_E;
    LCD_NOP; 
    CLR_LCD_E;
    LCD_NOP;
    _delay_us(80);

    WriteToLCD (0x28 , LCDCOMMAND) ;
    LCD_DISPLAY(0) ;
    LCD_CLEAR ;
    LCD_ENTRY_MODE(LCDINCREMENT) ;
}


/*--------------------------------------------------------*/
/* Wyswietla tekst na aktualnej pozycji kursora */

void lcd_puts(char *str)
{
    unsigned char i =0;

    while( str[i])
        LCD_WRITE_DATA(str[i++]) ;
}

//przeci¹¿y³em tê funkcjê, bo autor tego nie zrobi³

void lcd_puts(const char *str)
{
    unsigned char i =0;

    while( str[i])
        LCD_WRITE_DATA(str[i++]) ;
}

void lcd_puts(int a)
{
	char tmp[3];
	
	itoa(a, tmp, 10);
	
	unsigned char i =0;
	while( tmp[i])
        LCD_WRITE_DATA(tmp[i++]) ;
}
