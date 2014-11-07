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
	UCSR0A|= (1<<U2X0); // Double speed
	
	DDRD = 0x04; //Sätt PD2 till utgång
	
	UCSR0B|= (1<<TXEN0)|(1<<RXEN0); //Aktivera sändare och mottagare
	UCSR0C|= (0<<UMSEL00)|(0<<UMSEL01)|(1<<UCSZ00)|(1<<UCSZ01); //Sätt till 8bits instruktioner och asynkront läge
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

void SendAX() {
	PORTD = (1<<PORTD2);
}

void ReceiveAX() {
	loop_until_bit_is_set(UCSR0A, TXC0); //Vänta tills transmitregistret är tomt.
	PORTD = (0<<PORTD2);
}

// Funktioner för att skicka och ta emot paket

void PushHeaderAX(byte id, byte len, byte inst) {
	//Pusha information till uart
	SendAX(); //Sätt i skicka-läge
	uart_putchar(0xFF);
	uart_putchar(0xFF);

	checksum = 0; //Räkna inte med de första två bitarna

	//uart_putchar(id);
	//uart_putchar(len + 2);
	//uart_putchar(inst);
	uart_putchar(1);
	uart_putchar(4);
	uart_putchar(1);
}

void PushFooterAX() {
	//CHECKSUM
	uart_putchar(~checksum);
	ReceiveAX(); //Sätt i receive-mode
}

 ResponsePacket RecieveHeaderAX() {
	/* 
		Paketformat: 
		0XFF 0XFF ID LENGTH ERROR PARAMETER1 PARAMETER2…PARAMETER N CHECKSUM
	*/
	// Ta emot 0xFF 0xFF
	uart_getchar();
	uart_getchar();

	volatile ResponsePacket res;
	
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
	//return;
}

void ReadAX(byte id, byte address, byte len) {
	PushHeaderAX(id, 2, INST_READ); //skicka header
	uart_putchar(address);
	uart_putchar(len);
	PushFooterAX(); //skicka footer
	
	// Ta emot svar
	ResponsePacket res = RecieveHeaderAX();
}