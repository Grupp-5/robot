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
		if(autoMode == 0) {//Change mode
			autoMode = 1;
		}else {
			autoMode = 0;
		}
		sendToBus(0);//Tell beslutsenhet to change mode
	}
}

void sendToBus(uint8_t command)
{
	
}

//Data received interrupt
ISR(USART1_RX_vect) {
	uint8_t command = UDR1;
	if(autoMode == 1) {
		if(command == 0) {//Change mode
			autoMode = 0;
			sendToBus(command);//Tell beslutsenhet to change mode
		}
	}else {	
		if(command == 0) {//Change mode
			autoMode = 1;
		}
		sendToBus(command);
	}
}

//Initialize the USART
void USART_Init() {
	UBRR1H = (BAUD_PRESCALE >> 8);//High register
	UBRR1L = BAUD_PRESCALE;//Low register
	
	//Set 8 databits
	UCSR0C|= (1<<UCSZ00)|(1<<UCSZ01);
	//Enable RX, TX
	UCSR1B |= ((1<<TXEN1)|(1<<RXEN1));	
}

uint8_t USART_Receive_Byte(void) {
	while(((UCSR1A) & (1<<RXC1)) == 0);//Wait for data
	return UDR1;//Return received data from buffer
}

void USART_Send_Byte(uint8_t data) {
	while((UCSR1A & (1<<UDRE1)) == 0);//Wait for empty transmit buffer
	UDR1 = data; //Puts the data into the buffer, sends the data
}

int main() {
	autoMode = 0;
	
	DDRD = (1<<DDD3)|(1<<DDD4);//Set pin directions
	PCMSK0 = (1<<PCINT0);//Change pin mask
	PCICR = (1<<PCIE0);//Enable pin change interrupts
	UCSR1B |= (1<<RXCIE1)//Enable USART receive interrupt
	USART_Init();//Initialize USART1
	sei();//Enable interrupts in status register
	
	//Main loop
    while(1) {

    }
}