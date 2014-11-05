/*
 * Comunication.c
 *
 * Created: 2014-11-04 10:15:58
 *  Author: erima694 & eribo740
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

//Set CPU clock
#define F_CPU 8000000UL

//Define properties for USART
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
volatile uint8_t autoMode;

//Interrupt routine for PCINT0
//Switch autoMode
//Active on falling edge
ISR(PCINT0_vect) {
	//Detect falling edge
	if((PINA & 0x01) == 0) {
		if(autoMode == 0) {
			autoMode = 1;
		}else {
			autoMode = 0;
		}
	}
}

//Initialize the USART
void USART_Init(void) {
	UBRR1H = (BAUD_PRESCALE >> 8);//High register
	UBRR1L = BAUD_PRESCALE;//Low register
	
	//Default frame format is 8 data bits, no parity and 1 stop bit
	
	//Enable RX, TX
	UCSR1B= ((1<<TXEN1)|(1<<RXEN1));	
}

uint8_t USART_Receive_Byte(void) {
	while(((UCSR1A) & (1<<RXC1)) == 0);//Wait for data
	return UDR1;//Return received data from buffer
}

void USART_Send_Byte(uint8_t data) {
	while((UCSR1A & (1<<UDRE1)) == 0);//Wait for empty transmit buffer
	UDR1 = data; //Puts the data into the buffer, sends the data
}

int main(void) {
	autoMode = 0;
	DDRD = (1<<DDD3)|(1<<DDD4);//Set pin directions
	PCMSK0 = (1<<PCINT0);//Change pin mask
	PCICR = (1<<PCIE0);//Enable pin change interrupts
	USART_Init();
	sei();//Enable interrupts in status register
	
    while(1) {
		while(autoMode == 0) {
			while((PIND & 0x02) == 1);//Wait for Firefly to want to send
			cli();//Disable interrupts in status register
			PIND = PIND & (~(1<<PINA4));//Ok to send
			uint8_t data = USART_Receive_Byte();
			PIND = PIND | (1<<PINA4);
			sei();
		}
		
		while(autoMode == 1);	
    }
}