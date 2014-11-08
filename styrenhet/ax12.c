/*
* Styrenheten
* Date : 2014.11.04
* Author : dansj232, geoza435
*/

#include <avr/io.h>
#include "ax12.h"
#include <util/delay.h>

//Globala variabler
byte checksum;

//Initiera uart
void uart_init() {
	UBRR0H = (DEFAULT_BAUD_RATE>>8); // Skifta registret höger 8 bitar
	UBRR0L = DEFAULT_BAUD_RATE;      // Sätt baud rate
	
	DDRD |= (1<<DDD2); //Sätt PD2 till utgång
	
	UCSR0B |= (1<<TXEN0)|(1<<RXEN0); // Aktivera sändare och mottagare
	
	/*
	"The Dynamixel actuators communicate through asynchronous serial communication
	with 8 bit, 1 stop bit and no parity."
	*/
	
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01); // Sätt till 8bits instruktioner
}


void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); //Vänta tills dataregistret är tomt.
	UDR0 = c;
	checksum += c;
}

char uart_getchar() {
	// TODO: Gör så att det blir en timeout efter ett tag
	loop_until_bit_is_set(UCSR0A, RXC0); // Vänta tills data existerar.
	return UDR0;
}

void SetSendAX() {
	PORTD |= (1<<PORTD2);
}

void SetReceiveAX() {
	//loop_until_bit_is_set(UCSR0A, TXC0); // Vänta tills data har skickats klart
	_delay_us(15); // Världens vackraste lösning
	
	PORTD &= ~(1<<PORTD2); // Sätt i läs-läge
}

// Funktioner för att skicka och ta emot paket
ResponsePacket ReceiveCmdAX() {
	/*
	Paketformat:
	0XFF 0XFF ID LENGTH ERROR PARAMETER1 PARAMETER2…PARAMETER N CHECKSUM
	*/
	
	SetReceiveAX();
	
	// Ta emot 0xFF 0xFF
	uart_getchar(); // Konstig nolla först
	uart_getchar();
	uart_getchar();

	volatile ResponsePacket res = {0};
	
	res.id = uart_getchar();
	res.length = uart_getchar();
	res.error = uart_getchar();
	
	for (int i = 0; i < res.length - 2; i++) {
		res.params[i] = uart_getchar();
	}
	
	res.checksum = uart_getchar();
	
	return res;
}

int SendCmdAX(byte id, byte inst, byte len, byte params[]) {
	/*
	Paketformat:
	0xFF 0xFF ID LENGTH INSTRUCTION PARAMETER1 …PARAMETER N CHECK SUM
	*/
	SetSendAX(); // Sätt i skicka-läge
	uart_putchar(0xFF);
	uart_putchar(0xFF);

	checksum = 0; // Räkna inte med de första två bitarna

	uart_putchar(id);
	uart_putchar(len);
	uart_putchar(inst);
	
	for(int c = 0; c < len - 2; c++) {
		uart_putchar(params[c]);
	}
	
	uart_putchar(~checksum);
	
	return 0;
}

// Instruktioner
ResponsePacket PingAX(byte id) {
	SendCmdAX(id, INST_PING, 2, 0);
	
	// Ta emot svar
	ResponsePacket res = ReceiveCmdAX();
	
	// TODO: Kolla efter fel
	return res;
}
