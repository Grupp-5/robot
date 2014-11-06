/*
* enkelt_test.c
*
* Created: 11/6/2014 7:46:37 PM
*  Author: geoza435
*/

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 8000000UL // Anv�nds f�r _delay_ms()
#endif                  // �r 8 MHz n�r SUT_CKSEL �r INTRCOSC_6CK_4MS1
// (Under Device Programming -> Fuses)

#include <util/delay.h> // delay, delay_ms och delay_us


/*
* Tanken �r att det ska ske ett pulst�g p� Port B (pin 1-8)
* med 50 ms h�g och 50ms l�g
*/
int main(void) {
	DDRB|= 0xFF; // Anv�nd hela Port B som utdata
	while (1)
	{
		PORTB |= 0xFF; // S�tt 1 p� alla pins
		_delay_ms(50); // V�nta i 50 ms
		
		PORTB &= 0x00; // s�tt 0 p� alla pins
		_delay_ms(50);
	}
}