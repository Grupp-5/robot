/*
* Styrenheten
* Date : 2014.11.04
* Author : Daniel
*/

#include "constants.c"


void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); //Vänta tills dataregistret är tomt, RÄTT REGISTER???
	UDR0 = c;
}

char uart_getchar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Vänta tills data existerar, RÄTT REGISTER???. */
    return UDR0;
}

void PortInitialize(void) {
	DDRA = DDRB = DDRC = DDRD = 0; //Sätt alla portar till input
	PORTA = PORTB = PORTC = PORTD = 0; //Sätt portdata till 0

	DDRD = 0x40; //Pin D1 output, resten input
}

// Funktioner för att skicka och ta emot paket

void PushHeaderAX(byte id, byte len, byte inst) {
	//Pusha information till uart
	//FF
	//FF
	//ID
	//LENGTH
	//INST
	//PARAM1
	//PARAM2
}

void PushFooterAX() {
	//CHECKSUM
}


// Instruktioner

void PingAX(byte id) {
	//skicka header
	//skicka footer
}