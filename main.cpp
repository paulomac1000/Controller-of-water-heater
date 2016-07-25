//#include "main.h" //wczytujê plik nag³ówkowy
#include "hd44780.h"	//obs³uga wyœwietlacza
#include "ds18b20.h"	//obs³uga termometru

//do podci¹gania napiêcia do portów PB
#define PORTPB1 (1 << 1)
#define PORTPB2 (1 << 2)
#define PORTPB3 (1 << 3)

//przyciski
#define tempplus (!(PINC & (1 << PC0)))
#define tempminus (!(PINC & (1 << PC1)))
#define mocplus (!(PINC & (1 << PC2)))
#define mocminus (!(PINC & (1 << PC3)))
#define pauza (!(PINC & (1 << PC4)))

//do leda/buzzera
#define zapal_led (PORTB |= _BV(1))
#define zgas_led (PORTB &= ~(_BV(1)))

//do podœwietlenia ekranu
#define podswietl_ekran (PORTB |= _BV(2))
#define zgas_ekran (PORTB &= ~(_BV(2)))

//do triaka
#define timer_start 235 //timer ma liczyæ do 20, 235+20=255
#define NAPIECIENAPB3ON (PORTB |= _BV(3))
#define NAPIECIENAPB3OFF (PORTB &= ~(_BV(3)))

//////////////////////////////////////////////////////////
//do eepromu zmienne musz¹ byc globalnie				//
EEMEM uint16_t EEPROM_TEMP;								//
EEMEM uint16_t EEPROM_MOC;								//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//globalne zmienne ustawieñ                             //
int ustawiona_moc = 50;									//
int ustawiona_temperatura = 42;							//
//nadajê wartosci standardowe							//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//to musi byc globalnie aby funkcja przerwañ to widzia³a//
volatile bool ktory_sygnal = 0;							//
volatile int stosunek_sygnalu[2] = {5, 5};				//
volatile bool ciagly_sygnal = false;					//
volatile bool pusty_sygnal = false;						//
volatile bool cieply = false;							//
//do termometru, musi byc globalnie						//
volatile int odczytana_temperatura = 0;					//
volatile int ilosc_przerwan = 0;						//
//////////////////////////////////////////////////////////

class Wyswietlacz
{
private:

public:
	Wyswietlacz(){}

	void inicjalizacja()
	{
		/* Funkcja inicjalizuje wyœwietlacz*/
		lcd_init();
		/* W³¹cza wyœwietlanie */
		LCD_DISPLAY(LCDDISPLAY);
		//opóŸnienie dla pewnoœci, ¿e wyœwietlacz sie zainicjalizowa³
		_delay_ms(100);

		LCD_LOCATE(0, 0); //1 kolumna 1 wiersz 
		lcd_puts("UST  XXC AKT XXC");
		_delay_ms(100);
		LCD_LOCATE(0, 1); //2 kolumna 2 wiersz 
		lcd_puts("MOC XXX% XXXXXXX");
		_delay_ms(100);
	}

	void wyswietl_ustawiona_temperature()
	{
		//podgrzewacz ma dzia³aæ w temperaturach <30;70>
		if (ustawiona_temperatura >= 30 && ustawiona_temperatura <= 70)
		{
			LCD_LOCATE(5, 0); //5 kolumna 1 wiersz

			lcd_puts(ustawiona_temperatura);
		}
		//je¿eli jakimœ cudem pojawi siê inna temperatura
		else
		{
			LCD_LOCATE(5, 0); //5 kolumna 1 wiersz
			lcd_puts("E!");
		}
	}

	void wyswietl_ustawiona_moc()
	{
		LCD_LOCATE(4, 1); //5 kolumna 2 wiersz

		if(ustawiona_moc == 0)
		{
			lcd_puts("  ");
			lcd_puts(ustawiona_moc);
		}
		else if (ustawiona_moc >= 10 && ustawiona_moc <= 90)
		{
			lcd_puts(" ");
			lcd_puts(ustawiona_moc);
		}
		else if (ustawiona_moc == 100)
		{
			lcd_puts(ustawiona_moc);
		}
		else
		{
			lcd_puts("Er!");
		}
	}

	void wyswietl_odczytana_temperature()
	{
		LCD_LOCATE(12, 0);
		lcd_puts(" ");
		
		//temperatura 1-cyfrowa
		if (odczytana_temperatura >= 0 && odczytana_temperatura <= 9)
		{
			lcd_puts("0");
			lcd_puts(odczytana_temperatura);
		}
		//temperatura dwucyfrowa
		else if (odczytana_temperatura >= 10 && odczytana_temperatura <= 99)
		{
			lcd_puts(odczytana_temperatura);
		}
		//temperatura trzycyfrowa
		else if (odczytana_temperatura >= 100 && odczytana_temperatura <= 999)
		{
			LCD_LOCATE(12, 0);
			lcd_puts(odczytana_temperatura);
		}
		//je¿eli temperatura jednocyfrowa ujemna
		else if (odczytana_temperatura <= -1 && odczytana_temperatura >= -9)
		{
			lcd_puts(odczytana_temperatura);
		}
		//je¿eli temperatura dwucyfrowa ujemna
		else if (odczytana_temperatura <= -10 && odczytana_temperatura >= -99)
		{
			LCD_LOCATE(12, 0);
			lcd_puts(odczytana_temperatura);
		}
		//je¿eli jakœ inna temperatura, której nie da siê obs³u¿yc
		else
		{
			lcd_puts("E!");
		}

		lcd_puts("C"); //bo czasami nadpisuje jakieœ cyfry na tym C
	}
	
	void wyswietl_stan(char* str)
	{
		LCD_LOCATE(9, 1); //10 kolumna 2 wiersz 
	
		lcd_puts(str);
	}
	
	
	~Wyswietlacz(){}
};

class Eeprom
{
private:
	int dotychczasowa_ustawiona_moc;
	int dotychczasowa_ustawiona_temperatura;
	
	void odczytaj_temperature()
	{
		int odczytana_temperatura_z_eeprom;
	
		odczytana_temperatura_z_eeprom = (int)eeprom_read_word(&EEPROM_TEMP);
		_delay_ms(200);

		dotychczasowa_ustawiona_temperatura = odczytana_temperatura_z_eeprom;

		if (odczytana_temperatura_z_eeprom >= 30 && odczytana_temperatura_z_eeprom <= 70)
		{
			ustawiona_temperatura = odczytana_temperatura_z_eeprom;
		}
		else
		{
			dotychczasowa_ustawiona_temperatura = 0;
			ustawiona_temperatura = 42;
			zapisz_temperature();
		}
	}
	
	void odczytaj_moc()
	{
			int odczytana_moc_z_eeprom;

			odczytana_moc_z_eeprom = (int)eeprom_read_word(&EEPROM_MOC);
			_delay_ms(200);

			dotychczasowa_ustawiona_moc = odczytana_moc_z_eeprom;

			if (odczytana_moc_z_eeprom >= 0 && odczytana_moc_z_eeprom <= 100)
			{
				ustawiona_moc = odczytana_moc_z_eeprom;
			}
			else
			{
				dotychczasowa_ustawiona_moc = 0;
				ustawiona_moc = 50;
				zapisz_moc();
			}
		}

public:

	Eeprom()
	{
		odczytaj_temperature();
		odczytaj_moc();
	}
	
	void zapisz_temperature()
	{

		if (ustawiona_temperatura != dotychczasowa_ustawiona_temperatura)
		{
			//przed zapisem nalezy zatrzymaæ timer
			cli();
			//upewniamy siê ze eeprom nie jest u¿ywany
			eeprom_busy_wait();
			//zapisujê - rzutujê bo eeprom_update_word obs³uguje tylko 16bitow¹ liczbê
			eeprom_update_word(&EEPROM_TEMP, (uint16_t)ustawiona_temperatura);
			//opóŸnienie dla pewnoœci
			_delay_ms(200);
			//wznawiam timer
			sei();
		}
	}
	
	void zapisz_moc()
	{
		if (ustawiona_moc != dotychczasowa_ustawiona_moc)
		{
			//przed zapisem nalezy zatrzymaæ timer
			cli();
			//upewniamy siê ze eeprom nie jest u¿ywany
			eeprom_busy_wait();
			//zapisujê
			eeprom_update_word(&EEPROM_MOC, (uint16_t)ustawiona_moc);
			//opóŸnienie dla pewnoœci
			_delay_ms(200);
			//wznawiam timer
			sei();
		}
	}
	
	~Eeprom(){}
};

class Praca
{
private:

	bool czy_pracuje;
	char stan[8];
	bool pamiec_czy_pracuje;

public:

	void ustaw_stosunek_sygnalu(int moc, Wyswietlacz &wyswietlacz)
	{
		pusty_sygnal = false;
		ciagly_sygnal = false;

		switch (moc)
		{
			case 0:
				stosunek_sygnalu[0] = 0;
				stosunek_sygnalu[1] = 0;
				pusty_sygnal = true;
				ciagly_sygnal = false;
				break;

			case 10:
				stosunek_sygnalu[0] = 1;
				stosunek_sygnalu[1] = 9;
				break;

			case 20:
				stosunek_sygnalu[0] = 2;
				stosunek_sygnalu[1] = 8;
				break;

			case 30:
				stosunek_sygnalu[0] = 3;
				stosunek_sygnalu[1] = 7;
				break;

			case 40:
				stosunek_sygnalu[0] = 4;
				stosunek_sygnalu[1] = 6;
				break;

			case 50:
				stosunek_sygnalu[0] = 5;
				stosunek_sygnalu[1] = 5;
				break;

			case 60:
				stosunek_sygnalu[0] = 6;
				stosunek_sygnalu[1] = 4;
				break;

			case 70:
				stosunek_sygnalu[0] = 7;
				stosunek_sygnalu[1] = 3;
				break;

			case 80:
				stosunek_sygnalu[0] = 8;
				stosunek_sygnalu[1] = 2;
				break;

			case 90:
				stosunek_sygnalu[0] = 9;
				stosunek_sygnalu[1] = 1;
				break;

			case 100:
				stosunek_sygnalu[0] = 10;
				stosunek_sygnalu[1] = 0;
				pusty_sygnal = false;
				ciagly_sygnal = true;
				break;

			default:

				LCD_LOCATE(9, 1); //10 kolumna 2 wiersz
				lcd_puts(" ERROR ");

				pusty_sygnal = true;

				_delay_ms(400);

			break;
		}
	}

	Praca()
	{
		czy_pracuje = false;
		strcpy(stan, " START ");
	}
	
	int getUstawiona_moc()
	{
		return ustawiona_moc;
	}

	int getUstawiona_temperatura()
	{
		return ustawiona_temperatura;
	}

	bool getCzy_pracuje()
	{
		return czy_pracuje;
	}

	char* getStan()
	{
		return stan;
	}

	void setCzy_pracuje(bool czy_pracuje)
	{
		this->czy_pracuje = czy_pracuje;
	}
	
	void grzej(Wyswietlacz &wyswietlacz)
	{
		if (czy_pracuje == false && cieply == false)
		{
			strcpy(stan, "PRACUJE"); 
			
			wyswietlacz.wyswietl_stan(stan);

			ustaw_stosunek_sygnalu(ustawiona_moc, wyswietlacz);

			czy_pracuje = true;
		}
	}
	
	void nie_grzej(Wyswietlacz &wyswietlacz)
	{
		if (czy_pracuje == true)
		{
			strcpy(stan, "CIEPLY!"); 
			
			wyswietlacz.wyswietl_stan(stan);

			pusty_sygnal = true;

			czy_pracuje = false;
		}
	}
	
	void zapauzuj(Wyswietlacz &wyswietlacz)
	{
		pamiec_czy_pracuje = czy_pracuje;

		czy_pracuje = false;

		pusty_sygnal = true;//puszczamy 0
		
		strcpy(stan, " PAUZA "); 

		wyswietlacz.wyswietl_stan(stan);
	}

	void odpauzuj(Wyswietlacz &wyswietlacz)
	{
		czy_pracuje = pamiec_czy_pracuje;

		pusty_sygnal = false; //informujemy ¿e ma nie leciec sygnal pusty

		ustaw_stosunek_sygnalu(ustawiona_moc, wyswietlacz);
	}

	~Praca(){}
};

class Przycisk
{
private:

public:
	Przycisk(){}
	
	void temperatura_plus(Wyswietlacz &wyswietlacz)
	{
		zapal_led;

		_delay_ms(400);

		if (ustawiona_temperatura < 70) //max temp to 70 stopni
		{
			ustawiona_temperatura = ustawiona_temperatura + 1;

			wyswietlacz.wyswietl_ustawiona_temperature();
		}

		_delay_ms(200);
			
		zgas_led;
	}
	
	void temperatura_minus(Wyswietlacz &wyswietlacz)
	{
		zapal_led;

		_delay_ms(400);

		if (ustawiona_temperatura > 30) //min temp to 30 stopni
		{

			ustawiona_temperatura = ustawiona_temperatura - 1;

			wyswietlacz.wyswietl_ustawiona_temperature();

		}
			
		_delay_ms(200);

		zgas_led;
	}
	
	void moc_plus(Wyswietlacz &wyswietlacz)
	{
		zapal_led;
			
		_delay_ms(400);

		if (ustawiona_moc < 100) //max moc to 100%
		{

			ustawiona_moc = ustawiona_moc + 10;

			wyswietlacz.wyswietl_ustawiona_moc();
				
		}
			
		_delay_ms(200);

		zgas_led;
	}
	
	void moc_minus(Wyswietlacz &wyswietlacz)
	{
		zapal_led;

		_delay_ms(400);

		if (ustawiona_moc > 0) //min moc to 0%
		{

			ustawiona_moc = ustawiona_moc - 10;

			wyswietlacz.wyswietl_ustawiona_moc();

		}
			
		_delay_ms(200);

		zgas_led;
	}
	
	void zapauzuj(Wyswietlacz &wyswietlacz, Praca &praca)
	{
		zapal_led;
		_delay_ms(400);

		praca.zapauzuj(wyswietlacz);
		zgas_led;

		bool pause = 1;
		do
		{
			if (pauza) //wciœniêty ponownie PC4 czyli pauza
			{
				zapal_led;
				_delay_ms(400);
				pause = 0;
			}
			_delay_ms(100);

			wyswietlacz.wyswietl_odczytana_temperature();

		} while (pause == 1);

		praca.odpauzuj(wyswietlacz);
			
		zgas_led;
	}
	
	~Przycisk(){}
};


void inicjalizacja_timer_0()
{
	TCCR0 |= (1 << CS00) | (1 << CS02); //preskaler 1024, czyli 1 jednostka = ok 1ms
	TCNT0 = timer_start; //wartoœæ pocz¹tkowa timera aby liczy³ do 20 jednostek

	NAPIECIENAPB3ON;
}

void wywolaj_odczyt_temperatury()
{
	ds18b20_ConvertT();
}

void odczytaj_temperature()
/*gotowy kod, od siebie doda³em tylko rzutowanie wyniku do intigera
{
	/* W tablicy zapisywane bêd¹ dane odczytane z uk³adu ds18b20 */
	unsigned char ds18b20_pad[9];

	/* Odczyt z uk³adu ds18b20, dane zapisywane s¹ w tablicy ds18b20_pad.
	Dwie pierwsze pozycje w tablicy to kolejno mniej znacz¹cy bajt i bardziej
	znacz¹cy bajt wartoœæ zmierzonej temperatury */
	ds18b20_Read(ds18b20_pad);

	/* Sk³ada dwa bajty wyniku pomiaru w ca³oœæ. Cztery pierwsze bity mniej
	znacz¹cego bajtu to czêœæ u³amkowa wartoœci temperatury, wiêc ca³oœæ
	dzielona jest przez 16 */
	odczytana_temperatura = (int)(((ds18b20_pad[1] << 8) + ds18b20_pad[0]) / 16.0);
}

void inicjalizacja_timer_2()
{
	TCCR2   |= (1 << CS22) | (1 << CS21) | (1 << CS20); //preskaler 1024, czyli 1 jednostka = ok 1ms
	TCNT1 = 155; //zaczyna od tej wartoœci aby doliczyc do 100ms
	wywolaj_odczyt_temperatury();
	_delay_ms(750);
}

void inicjalizacja_TIMSK()
{
	TIMSK |= (1 << TOIE0) | (1 << TOIE2);  //przerwanie gdy licznik przepe³niony
}

int main()
{

	//////////////////
	//    Porty B	//
	//				//
	//	PB1 = led	//
	//	PB2 = podsw.//
	//			lcd	//
	//	PB3 = wyjscie/
	//		triaka	//
	//	PB3 = BRAK	//
	//	PB4 = BRAK	//
	//	PB5 = BRAK	//
	//				//
	//////////////////

	DDRB |= PORTPB1; //tutaj bêdzie lampka
	DDRB |= PORTPB2; //tutaj bêdzie podœwietlenie wyœwietlacza
	DDRB |= PORTPB3; //tutaj bêdzie wyjœcie do triaka

	//////////////////
	//    Porty C	//
	//				//
	//	PC0 = temp+	//
	//	PC1 = temp-	//
	//	PC2 = moc+	//
	//	PC3 = moc-	//
	//	PC4 = pauza	//
	//	PC5 = BRAK	//
	//	PC6 = BRAK	//
	//				//
	//////////////////

	DDRC = 0x00; //wszystkie C jako wejœcia
	PORTC = 0xff; //podci¹gamy wszystkie C do napiêcia

	//////////////////
	//    Porty D	//
	//				//
	// prawie		//
	// wszystkie do //
	// wyœwietlacza //
	//				//
	//	PD0 = RS	//
	//	PD1 = RW	//
	//	PD2 = E		//
	//	PD3 = D4	//
	//	PD4 = D5	//
	//	PD5 = D6	//
	//	PD6 = D7	//
	//	PD7 = BRAK	//
	//				//
	// konfiguracja	//
	// w hd44780.h	//
	//////////////////

	////////////////////////////////////////////
	////////////////////////////////////////////

	Wyswietlacz wyswietlacz;
	wyswietlacz.inicjalizacja();
	
	Eeprom eeprom;
	wyswietlacz.wyswietl_ustawiona_temperature();
	wyswietlacz.wyswietl_ustawiona_moc();
	
	Praca praca;
	Przycisk przycisk;

	int licznik_iteracji = 0;
	bool czy_podswietlony = false;

	inicjalizacja_timer_0();
	inicjalizacja_timer_2();
	inicjalizacja_TIMSK();
	sei();

	//pêtla g³ówna
	while(1)
	{
		wyswietlacz.wyswietl_odczytana_temperature();

		//nas³uchujê wciœniêcia przycisków
		if(tempplus)
		{
			licznik_iteracji = 0;
			podswietl_ekran;
			czy_podswietlony = true;
			przycisk.temperatura_plus(wyswietlacz);
		}
		
		if(tempminus)
		{
			licznik_iteracji = 0;
			podswietl_ekran;
			czy_podswietlony = true;
			przycisk.temperatura_minus(wyswietlacz);
		}

		if(mocplus)
		{
			licznik_iteracji = 0;
			podswietl_ekran;
			czy_podswietlony = true;
			przycisk.moc_plus(wyswietlacz);
			praca.ustaw_stosunek_sygnalu(ustawiona_moc, wyswietlacz);
		}
		
		if(mocminus)
		{
			licznik_iteracji = 0;
			podswietl_ekran;
			czy_podswietlony = true;
			przycisk.moc_minus(wyswietlacz);
			praca.ustaw_stosunek_sygnalu(ustawiona_moc, wyswietlacz);
		}
		
		if(pauza)
		{
			licznik_iteracji = 0;
			podswietl_ekran;
			czy_podswietlony = true;
			przycisk.zapauzuj(wyswietlacz, praca);
		}
		
		//zale¿nie od odczytanej temperatury ustawiam czy grzaæ czy nie
		if (odczytana_temperatura < ustawiona_temperatura)
		{
			cieply = false;
			praca.grzej(wyswietlacz);
		}
		else if (odczytana_temperatura >= ustawiona_temperatura)
		{
			cieply = true;
			praca.nie_grzej(wyswietlacz);
		}

		////////////////////////////
		//obs³uga zgaszania ekranu//
		//obs³uga zapisu do eeprom//
		////////////////////////////

		//je¿eli ekran jest poœwietlony to liczy iteracje
		//g³ównej pêtli, ekran zostanie zgaszony po 10k itaracjach
		//po zgaszeniu zast¹pi zapis do eeprom
		if (czy_podswietlony == true)
		{
			licznik_iteracji++;

			//ekran jest podœwietlony przez 10k iteracji
			if(licznik_iteracji == 10000)
			{
				licznik_iteracji = 0;
				zgas_ekran;
				czy_podswietlony = false;
				//po zgaszeniu ekranu dokunujê zapisu do eeprom
				//funkcja za to odpowiedzialna sprawdza czy zapisac
				//czy ju¿ taka wartoœc by³a ostatnio w eeprom
				eeprom.zapisz_temperature();
				eeprom.zapisz_moc();
			}
		}

	}

	return 0;
}


//obs³uga przerwania do triaka
ISR(TIMER0_OVF_vect)
{
	if (ciagly_sygnal == true)
	{
		ktory_sygnal = 1;
		NAPIECIENAPB3ON;
	}
	else if (pusty_sygnal == true)
	{
		ktory_sygnal = 0;
		NAPIECIENAPB3OFF;
	}
	else if (cieply == false)
	{
		if (ktory_sygnal == 1) //jest sygna³
		{
			NAPIECIENAPB3ON;
			ktory_sygnal = 0;
			TCNT0 = 255	- stosunek_sygnalu[0]*20;
		}
		else //nie ma sygna³u
		{
			NAPIECIENAPB3OFF;
			ktory_sygnal = 1;
			TCNT0 = 255	- stosunek_sygnalu[1]*20;
		}
	}
}

//obs³uga przerwania do odczytu temperatury co 800ms
//jedno przerwanie = 100ms
ISR(TIMER2_OVF_vect)
{
	ilosc_przerwan++;

	if (ilosc_przerwan == 8)
	{
		odczytaj_temperature();
		wywolaj_odczyt_temperatury();
		ilosc_przerwan = 0;
	}
}
