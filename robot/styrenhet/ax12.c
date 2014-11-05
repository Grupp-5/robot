/*
* Styrenheten
* Date : 2014.11.04
* Author : Daniel
*/

#include "ax12.h"

//Globala variabler
byte checksum;

//Initiera uart
void uart_init (void) {
	UBRRH = (DEFAULT_BAUD_RATE>>8); //Skifta registret höger 8 bitar
	UBRRL = DEFAULT_BAUD_RATE; //Sätt baud rate
	UCSRB|= (1<<TXEN)|(1<<RXEN); //Aktivera sändare och mottagare
	UCSRC|= (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1); //Sätt til 8bits instruktioner
}


void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); //Vänta tills dataregistret är tomt, RÄTT REGISTER???
	UDR0 = c;
	checksum += c;
}

char uart_getchar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Vänta tills data existerar, RÄTT REGISTER???. */
    return UDR0;
}

void PortInitialize(void) {
	DDRA = DDRB = DDRC = DDRD = 0; //Sätt alla portar till input
	PORTA = PORTB = PORTC = PORTD = 0; //Sätt portdata till 0
}

//Initiera uart
void uart_init (void) {
	UBRRH = (DEFAULT_BAUD_RATE>>8); //Skifta registret höger 8 bitar
	UBRRL = DEFAULT_BAUD_RATE; //Sätt baud rate
	UCSRB|= (1<<TXEN)|(1<<RXEN); //Aktivera sändare och mottagare
	UCSRC|= (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1); //Sätt til 8bits instruktioner
}

// Funktioner för att skicka och ta emot paket

void PushHeaderAX(byte id, byte len, byte inst) {
	//Pusha information till uart
	uart_putchar(0xFF);
	uart_putchar(0xFF);

	checksum = 0; //Räkna inte med de första två bitarna

	uart_putchar(id);
	uart_putchar(len + 2);
	uart_putchar(inst);
}

void PushFooterAX() {
	//CHECKSUM
	uart_putchar(~checksum);
}


// Instruktioner

void PingAX(byte id) {
	PushHeaderAX(id, 2, AX_INST_PING); //skicka header
	PushFooterAX(); //skicka footer
}