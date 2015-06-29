/*
 * mkuart.h
 *
 *  Created on: 2010-09-04
 *       Autor: Miros³aw Kardaœ
 */

#ifndef MKUART_H_
#define MKUART_H_


#define UART_BAUD 9600		// tu definiujemy interesuj¹c¹ nas prêdkoœæ
#define __UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1) // obliczamy UBRR dla U2X=0

// deklaracje funkcji publicznych

void UART_Init( uint16_t UBRR );
void uart_putc( char data );
void uart_puts(char *s);
void uart_puts_P(const char *s);
void uart_putint( uint32_t liczba, uint8_t radix);

#endif /* MKUART_H_ */
