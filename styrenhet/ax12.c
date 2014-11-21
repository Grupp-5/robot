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
	
	DDRD |= (1<<DDD2)|(1<<DDD3); //Sätt PD2 och PD3 till utgång
	// PD3 används för att visa UART Timeout
	
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
	
	PORTD &= ~(1<<PORTD3);
	
	// Vänta tills data existerar. Eller om det tar för lång tid, skicka
	// tillbaka ett error.
	// TODO: Satte bara 50000 för att det kändes bra. Timeouts? Konstant nånstans?
	uint16_t i=500;
	do {
		if (UCSR0A & (1<<RXC0)) return UDR0;
	} while(--i);
	
	// TODO: Set global error instead
	PORTD |= (1<<PORTD3);
	// TODO: Oklar konstant. FE som i FEl.
	return 0xFE;
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

//============ INSTRUKTIONER ===================

//PING
ResponsePacket PingAX(byte id) {
	SendCmdAX(id, INST_PING, 2, 0);
	
	// Ta emot svar
	ResponsePacket res = ReceiveCmdAX();
	
	// TODO: Kolla efter fel
	return res;
}

ResponsePacket ReadAX(byte id, byte address, byte length) {
	byte params[2] = {address, length};
	SendCmdAX(id, INST_READ, 2+2, params);
	
	// Ta emot svar
	ResponsePacket res = ReceiveCmdAX();
	
	// TODO: Kolla efter fel
	return res;
}

// Skriv till ett 8-bitars register
ResponsePacket Write8AX(byte id, byte adr, uint8_t value, bool reg) {
	byte params[2] = {adr, value};
	SendCmdAX(id, reg ? INST_REG_WRITE : INST_WRITE, 2+2, params);

	// TODO: Receive-mode är ansvarig för att stänga av "send mode"
	ResponsePacket res;
	if (id != ID_BROADCAST) {
		// TODO: Förutsatt att AX_RETURN_LEVEL = 2
		// Ta emot svar
		res = ReceiveCmdAX();
	}

	// TODO: Kolla efter fel
	return res;
}

// Skriv till ett 16-bitars register
ResponsePacket Write16AX(byte id, byte adr, uint16_t value, bool reg) {
	byte params[3] = {adr, value, value >> 8};
	SendCmdAX(id, reg ? INST_REG_WRITE : INST_WRITE, 2+3, params);

	// TODO: Receive-mode är ansvarig för att stänga av "send mode"
	ResponsePacket res;
	if (id != ID_BROADCAST) {
		// TODO: Förutsatt att AX_RETURN_LEVEL = 2
		// Ta emot svar
		res = ReceiveCmdAX();
	}
	
	// TODO: Kolla efter fel
	return res;
}

ResponsePacket ActionAX(byte id) {
	SendCmdAX(id, INST_ACTION, 2, 0);
	
	// Ta emot svar
	ResponsePacket res = ReceiveCmdAX();
	
	// TODO: Kolla efter fel
	return res;
}

ResponsePacket ReadAllAX(byte id) {
	return ReadAX(id, 0, AX_PUNCH_H+1); // Sista platsen i minnet
}
