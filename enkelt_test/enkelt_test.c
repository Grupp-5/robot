/*
* enkelt_test.c
*
* Created: 11/6/2014 7:46:37 PM
*  Author: geoza435
*/

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 8000000UL // Används för _delay_ms()
#endif                  // Är 8 MHz när SUT_CKSEL är INTRCOSC_6CK_4MS1
// (Under Device Programming -> Fuses)

#include <util/delay.h> // delay, delay_ms och delay_us


/*
* Tanken är att det ska ske ett pulståg på Port B (pin 1-8)
* med 50 ms hög och 50ms låg
*/
int main(void) {
	DDRB|= 0xFF; // Använd hela Port B som utdata
	while (1)
	{
		PORTB |= 0xFF; // Sätt 1 på alla pins
		_delay_ms(50); // Vänta i 50 ms
		
		PORTB &= 0x00; // sätt 0 på alla pins
		_delay_ms(50);
	}
}