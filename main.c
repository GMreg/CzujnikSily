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
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ USTAWIENIA WSTÊPNE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//ustawienie ADC
	ADCSRA |= (1<<ADEN); // w³¹cz ADC
	ADCSRA |= (1<<ADPS2); // preskaler = 16
	ADMUX = REF_VCC; // ustawiamy wewn Ÿród³o odniesienia
	//koniec ustawieñ ADC

	//przygotowanie przycisku na PD2 do u¿ycia
	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2);
	//koniec przygotowania przycisku

	//inicjalizacja UART
	UART_Init( __UBRR );	// inicjalizacja UART
	//koniec inicjalizacji UART

	sei(); // zgoda na przerwania
	char zezwolenie = 0; 	//zmienna zezwolenie jest na potrzeby przycisku, po wciœniêciu ustawia siê na 0 i blokuje ponowne wciœniêcie,
							//natomiast po puszczeniu ustawia siê na 1 i pozwala na ponowne u¿ycie przycisku

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ KONIEC USTAWIEÑ WSTÊPNYCH ~~~~~~~~~~~~~~~~~~~~~~~~




	while(1){

		if(pomiar(5)>18){ //minimalne napiêcie z czujnika to oko³o 91mV, ten if zabezpiecza przed pikami poni¿ej tej wartoœci

			licznik2=pomiar(5);//wykonanie pomiaru i nadpisanie zmiennej licznik2

			for(int juuu=1; juuu<=10; juuu++){ //ta pêtla sprawdza czy wykonany pomiar jest wiêkszy od którejœ wartoœci w tablicy i zastêpuje j¹

				if(licznik2>tablica[juuu]){

					tablica[juuu]=licznik2;
					juuu=10; // jak wejdzie do tego ifa, to wyskakuje z pêtli, nie przeszukuje jej dalej czyli wybiera pierwsz¹ napotkan¹

				}
			}


			SUMA=0; //zerowanie SUMY z poprzedniego pomiaru
			for(int juuu=0; juuu<=10; juuu++){ // sumowanie ca³ej tablicy

				SUMA=SUMA+tablica[juuu];

			}

			SUMA=SUMA/10; // podzielenie ca³ej tablicy przez iloœ elementów (œrednia)
			SUMA_pomoc=SUMA*5/1023; // przeliczenie uœrednionego ADC na V
			SUMAmV=SUMA_pomoc*1000; // przejscie na mV
			SUMA_pomoc=SUMAmV*26.91 - 2449; // przeliczenie tego wszystkiego si³ê

			//obs³uga UARTA, wypisanie wyników
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


		if(!( PIND & (1<<PD2) )){//obs³uga przycisku, po naciœniêciu mozna wykonac kolejny pomiar

			_delay_ms(20); // opóŸnienie, a po nim kolejne sprawdzenie przycisku, w celu wyeliminowania drgañ
			if(!(PIND & (1<<PD2)) && zezwolenie==1){ // sprawdza czy przycisk jest wciœniêty i czy jest zezwolenie

				for(int juuu=0; juuu<=10; juuu++){

					tablica[juuu]=0;
					licznik2=0;
					SUMA=0;
					zezwolenie = 0; // ustawienie braku zezwolenia na kolejne wejœcie do tego ifa

				}

			}
		}//koniec obs³ugi przycisku

		if(PIND & (1<<PD2)){ //gdy przyciski nie s¹ aktywne, ustawiane jest zezwolenie

				zezwolenie = 1; //ma to na celu wyeliminowanie efektu podwójnego klikniêcia

		}


	}// koniec g³ównej pêtli while
}//koniec main()





// wykonywanie pomiaru adc
uint16_t pomiar( uint8_t kanal ){
	ADMUX = (ADMUX & 0b11111000) | kanal;
	ADCSRA |= (1<<ADSC); //start konwersji
	return ADCW;
}
// koniec wykonywania pomiaru adc
