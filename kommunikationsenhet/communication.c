/*
 * Comunication.c
 *
 * Created: 2014-11-04 10:15:58
 *  Author: erima694 & eribo740
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <common.h>
#include <modulkom.h>
#include "Communication.h"

//Set CPU clock
#define F_CPU 14745600UL

//Define properties for USART
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
volatile uint8_t autoMode;
volatile uint8_t waitForP;
volatile uint8_t waitForD;

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
Bus_data master_data_to_send = {0};
Bus_data master_data_to_receive = {0};
	
void send_to_bus(Device_id dev_id, Data_id data_id, uint8_t arg_count, uint8_t data_array[]) {
	master_data_to_send.id = data_id;
	master_data_to_send.count = arg_count+2;
	for(int i = 0; i<arg_count; i++) {
		master_data_to_send.data[i] = data_array[i];
	}
	send_data(dev_id, master_data_to_send);
}

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
		uint8_t commands[1] = {CHANGEMODE};
		send_to_bus(DECISION, COMMAND_DATA, 1, commands);//Tell beslutsenhet to change mode
	}
}


//Data received interrupt
ISR(USART1_RX_vect) {
	uint8_t commands[1] = {UDR1};
	if(autoMode == 1) {
		if(commands[0] == CHANGEMODE) {//Change mode
			autoMode = 0;
			send_to_bus(DECISION, COMMAND_DATA, 1, commands);//Tell beslutsenhet to change mode
		}
	}else {	
		if(waitForP == 1) {
			waitForP = 0;
			send_to_bus(DECISION, P_DATA, 1, commands);//Send D value to beslutsenhet
		}else if(waitForD == 1) {
			waitForD = 0;
			send_to_bus(DECISION, D_DATA, 1, commands);//Send P value to beslutsenhet
		}else if(commands[0] == CHANGEMODE) {//Change mode
			autoMode = 1;		
			send_to_bus(DECISION, COMMAND_DATA, 1, commands);//Tell beslutsenhet to change mode
		}else if(commands[0] == WAIT_FOR_P) {
			waitForP = 1;
		}else if(commands[0] == WAIT_FOR_D) {
			waitForD = 1;
		}else {
			send_to_bus(CONTROL, COMMAND_DATA, 1, commands);
		}
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
	loop_until_bit_is_set(UCSR1A,RXC1);//Wait for data
	return UDR1;//Return received data from buffer
}

void USART_Send_Byte(uint8_t sendData) {
	loop_until_bit_is_set(UCSR1A,UDRE1);//Wait for empty transmit buffer
	UDR1 = sendData; //Puts the data into the buffer, sends the data
}

Bus_data prepare_data() {
	return data_to_send;
}

void interpret_data(Bus_data data){
	data_to_receive = data;
	if(data_to_receive.data[0] == STOP_TIMER) {
		autoMode = 0;
		USART_Send_Byte(data_to_receive.data[0]);
	}else if(data_to_receive.data[0] == START_TIMER) {
		USART_Send_Byte(data_to_receive.data[0]);
	}else {
		USART_Send_Byte(*data_to_receive.data);
	}
}

int main(void) {
	set_as_slave(prepare_data, interpret_data, COMMUNICATION);
	set_as_master(F_CPU);
	
	autoMode = 0;
	waitForP = 0;
	waitForD = 0;
	DDRD = (1<<DDD3)|(1<<DDD4);//Set pin directions
	PCMSK0 = (1<<PCINT0);//Change pin mask
	PCICR = (1<<PCIE0);//Enable pin change interrupts
	UCSR1B |= (1<<RXCIE1);//Enable USART receive interrupt
	USART_Init();//Initialize USART1
	sei();//Enable interrupts in status register
    while(1) {
		
    }
}
