/*
 * slave_test2.c
 *
 * Created: 11/20/2014 2:56:20 PM
 *  Author: emiva760, carth567
 */ 


#include <avr/io.h>
#include <modulkom.h>
#include <common.h>
#include <avr/interrupt.h>

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
	
Bus_data prepare_data() {
	return data_to_send;
}

void interpret_data(Bus_data data){
	data_to_receive = data;
}

int main(void)
{
	data_to_send.id = SENSOR_DATA;
	data_to_send.count = 4;
	data_to_send.data[0] = 20;
	data_to_send.data[1] = 42;
	data_to_send.data[2] = 255;
	
	PORTB = 0;
	sei();
	
	set_as_slave(prepare_data, interpret_data, SENSOR);
	
    while (1) {} 
}