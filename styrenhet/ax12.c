/*
* Styrenheten
* Date : 2014.11.04
* Author : Daniel
*/

#include <avr/io.h>
#include "ax12.h"

//Globala variabler
byte checksum;

//Initiera uart
void uart_init() {
	UBRR0H = (DEFAULT_BAUD_RATE>>8); //Skifta registret höger 8 bitar
	UBRR0L = DEFAULT_BAUD_RATE; //Sätt baud rate
	UCSR0B|= (1<<TXEN0)|(1<<RXEN0); //Aktivera sändare och mottagare
	UCSR0C|= (1<<UCSZ00)|(1<<UCSZ01); //Sätt til 8bits instruktioner
}


void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); //Vänta tills dataregistret är tomt.
	UDR0 = c;
	checksum += c;
}

char uart_getchar() {
	loop_until_bit_is_set(UCSR0A, RXC0); /* Vänta tills data existerar. */
	return UDR0;
}

void PortInitialize() {
	DDRA = DDRB = DDRC = DDRD = 0; //Sätt alla portar till input
	PORTA = PORTB = PORTC = PORTD = 0; //Sätt portdata till 0
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

 ResponsePacket RecieveHeaderAX() {
	/* 
		Paketformat: 
		0XFF 0XFF ID LENGTH ERROR PARAMETER1 PARAMETER2…PARAMETER N CHECKSUM
	*/
	// Ta emot 0xFF 0xFF
	uart_getchar();
	uart_getchar();

	ResponsePacket res;
	
	res.id = uart_getchar();
	res.error = uart_getchar();
	res.length = uart_getchar();
	
	for (int i = 0; i < res.length - 2; i++) {
		res.params[i] = uart_getchar();
	}
	res.checksum = uart_getchar();
	
	return res;
}

// Instruktioner

void PingAX(byte id) {
	PushHeaderAX(id, 2, INST_PING); //skicka header
	PushFooterAX(); //skicka footer
	
	// Ta emot svar
	ResponsePacket res = RecieveHeaderAX();
}