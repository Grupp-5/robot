/*
 * robot.c
 *
 * Created: 11/4/2014 9:35:08 AM
 *  Author: emiva760
 */ 


#include <avr/io.h>
#include "styrenhet/ax12.h"

int main(void)
{
	uart_init();
    while(1)
    {
		PingAX(3);
    }
}