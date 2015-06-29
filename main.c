/*
 * main.c
 *
 *  Created on: 20 mar 2015
 *      Author: Michal
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "MKUART/mkuart.h"


volatile uint16_t licznik2=0;
volatile double SUMA=0;
volatile double SUMAmV=0;
volatile double SUMA_pomoc=0;
volatile uint16_t tablica[10];

uint16_t pomiar (uint8_t kanal); // 16 bitowa zmienna pomiar na potrzeby ADC

#define REF_256 (1<<REFS1)|(1<<REFS0)
#define REF_VCC (1<<REFS0)

int main(void){
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ USTAWIENIA WST�PNE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//ustawienie ADC
	ADCSRA |= (1<<ADEN); // w��cz ADC
	ADCSRA |= (1<<ADPS2); // preskaler = 16
	ADMUX = REF_VCC; // ustawiamy wewn �r�d�o odniesienia
	//koniec ustawie� ADC

	//przygotowanie przycisku na PD2 do u�ycia
	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2);
	//koniec przygotowania przycisku

	//inicjalizacja UART
	UART_Init( __UBRR );	// inicjalizacja UART
	//koniec inicjalizacji UART

	sei(); // zgoda na przerwania
	char zezwolenie = 0; 	//zmienna zezwolenie jest na potrzeby przycisku, po wci�ni�ciu ustawia si� na 0 i blokuje ponowne wci�ni�cie,
							//natomiast po puszczeniu ustawia si� na 1 i pozwala na ponowne u�ycie przycisku

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ KONIEC USTAWIE� WST�PNYCH ~~~~~~~~~~~~~~~~~~~~~~~~




	while(1){

		if(pomiar(5)>18){ //minimalne napi�cie z czujnika to oko�o 91mV, ten if zabezpiecza przed pikami poni�ej tej warto�ci

			licznik2=pomiar(5);//wykonanie pomiaru i nadpisanie zmiennej licznik2

			for(int juuu=1; juuu<=10; juuu++){ //ta p�tla sprawdza czy wykonany pomiar jest wi�kszy od kt�rej� warto�ci w tablicy i zast�puje j�

				if(licznik2>tablica[juuu]){

					tablica[juuu]=licznik2;
					juuu=10; // jak wejdzie do tego ifa, to wyskakuje z p�tli, nie przeszukuje jej dalej czyli wybiera pierwsz� napotkan�

				}
			}


			SUMA=0; //zerowanie SUMY z poprzedniego pomiaru
			for(int juuu=0; juuu<=10; juuu++){ // sumowanie ca�ej tablicy

				SUMA=SUMA+tablica[juuu];

			}

			SUMA=SUMA/10; // podzielenie ca�ej tablicy przez ilo� element�w (�rednia)
			SUMA_pomoc=SUMA*5/1023; // przeliczenie u�rednionego ADC na V
			SUMAmV=SUMA_pomoc*1000; // przejscie na mV
			SUMA_pomoc=SUMAmV*26.91 - 2449; // przeliczenie tego wszystkiego si��

			//obs�uga UARTA, wypisanie wynik�w
			uart_puts("POMIAR F = ");
			uart_putint(SUMA_pomoc, 10);
			uart_puts("mN, czyli: ");
			uart_putint(SUMA_pomoc/1000, 10);
			uart_puts(" N | POMIAR ADC = ");
			uart_putint(SUMA, 10);
			uart_puts(", czyli: ");
			uart_putint(SUMAmV, 10);
			uart_puts("mV");
			uart_putc('\r');
			uart_putc('\n');

		}
		else{
			uart_puts("Napiecie ponizej 91mV. Podlacz czujnik");
			uart_putc('\r');
			uart_putc('\n');
		}


		if(!( PIND & (1<<PD2) )){//obs�uga przycisku, po naci�ni�ciu mozna wykonac kolejny pomiar

			_delay_ms(20); // op�nienie, a po nim kolejne sprawdzenie przycisku, w celu wyeliminowania drga�
			if(!(PIND & (1<<PD2)) && zezwolenie==1){ // sprawdza czy przycisk jest wci�ni�ty i czy jest zezwolenie

				for(int juuu=0; juuu<=10; juuu++){

					tablica[juuu]=0;
					licznik2=0;
					SUMA=0;
					zezwolenie = 0; // ustawienie braku zezwolenia na kolejne wej�cie do tego ifa

				}

			}
		}//koniec obs�ugi przycisku

		if(PIND & (1<<PD2)){ //gdy przyciski nie s� aktywne, ustawiane jest zezwolenie

				zezwolenie = 1; //ma to na celu wyeliminowanie efektu podw�jnego klikni�cia

		}


	}// koniec g��wnej p�tli while
}//koniec main()





// wykonywanie pomiaru adc
uint16_t pomiar( uint8_t kanal ){
	ADMUX = (ADMUX & 0b11111000) | kanal;
	ADCSRA |= (1<<ADSC); //start konwersji
	return ADCW;
}
// koniec wykonywania pomiaru adc
