/*
 * Comunication.c
 *
 * Created: 2014-11-04 10:15:58
 *  Author: erima694, eribo740, geoza435
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <common.h>
#include <modulkom.h>
#include "Communication.h"
#include "queue.h"

//Set CPU clock
#define F_CPU 14745600UL

//Define properties for USART
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
volatile uint8_t autoMode = 0;
volatile uint8_t waitForP;
volatile uint8_t waitForD;
volatile uint8_t poll;

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
Bus_data master_data_to_send = {0};
volatile Bus_data master_data_to_receive = {0};
	
void send_to_bus(Device_id dev_id, Data_id data_id, uint8_t data_array[]) {
	master_data_to_send.id = data_id;
	master_data_to_send.count = command_lengths[data_id]+2;
	for(int i = 0; i < command_lengths[data_id]; i++) {
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
		uint8_t data[1] = {autoMode};
		sei(); // TODO: Flytta ut till main-loop
		send_to_bus(which_device[CHANGEMODE], CHANGEMODE, data);//Tell beslutsenhet to change mode
	}
}

queue from_bt_buf;
queue to_bt_buf;

//Data received interrupt
ISR(USART1_RX_vect) {
	uint8_t command = UDR1;
	uint8_t data[MAX_DATA] = {0};
	// Tillverka en Bus_data manuellt
	data[0] = command_lengths[command] + 2;
	data[1] = command;
	// Vänta på mer data om det är vettigt
	for (uint8_t c = 2; c < command_lengths[command] + 2; c++) {
		loop_until_bit_is_set(UCSR1A, RXC1);
		data[c] = UDR1;
	}
	// Lägg till i kö som avverkas i main-loop
	enqueue(&from_bt_buf, *(Bus_data*) &data);
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
	if(data_to_receive.id == STOP_TIMER) {
		autoMode = 0;
	}
	enqueue(&to_bt_buf, data_to_receive);
}

//Poll for data
ISR(TIMER1_OVF_vect) {
	TCNT1H = 0x80; //Reset Timer1 high register
	TCNT1L = 0x00; //Reset Timer1 low register
	poll = 1;
}

//Initialize the timer interrupt to happen
//approximately once per second
void initTimer(void) {
	TIMSK1 = (1<<TOIE1);//Enable timer overflow interrupt for Timer1
	TCNT1H = 0x80; //Initialize Timer1 high register
	TCNT1L = 0x00; //Initialize Timer1 low register
	TCCR1B = (1<<CS11)|(1<<CS10);//Use clock/64 prescaler
}

int main(void) {
	set_as_slave(F_CPU, prepare_data, interpret_data, COMMUNICATION);
	set_as_master(F_CPU);
	
	poll = 0;
	autoMode = 0;
	waitForP = 0;
	waitForD = 0;
	DDRD = (1<<DDD3)|(1<<DDD4);//Set pin directions
	PCMSK0 = (1<<PCINT0);//Change pin mask
	PCICR = (1<<PCIE0);//Enable pin change interrupts
	UCSR1B |= (1<<RXCIE1);//Enable USART receive interrupt
	USART_Init();//Initialize USART1
	initTimer();

	Bus_data data;

	init_queue(&to_bt_buf);
	init_queue(&from_bt_buf);

	sei();//Enable interrupts in status register
    while(1) {
		if (poll == 1) {
			data_to_receive.count = command_lengths[SENSOR_DATA] + 2;
			fetch_data(SENSOR, &data_to_receive);
			enqueue(&to_bt_buf, data_to_receive);
			poll = 0;
		}

		// TODO: Inte säker på om det är vettigt med en loop här.
		while (!empty(&to_bt_buf)) {
			data = dequeue(&to_bt_buf);
			USART_Send_Byte(data.id);
			// Skicka resten av data, exklusive count och id
			for (uint8_t c = 0; c < data.count - 2; c++) {
				USART_Send_Byte(data.data[c]);
			}
		}

		while (!empty(&from_bt_buf))
		{
			data = dequeue(&from_bt_buf);
			// Skicka vidare kommandot
			send_data(which_device[data.id], data);
			if (data.id == STOP_TIMER) {
				autoMode = false;
			}
		}
    }
}
