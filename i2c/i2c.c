/*
 * i2c.c
 *
 * Created: 11/14/2014 12:05:29 PM
 *  Author: emiva760
 */

#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "i2c.h"

#define ERROR 1


void slave_init(uint8_t slave_address) {
	TWAR = (slave_address << 1);

	TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
}

void send_start_condition(void) {

	// Skicka startvillkor
	TWCR = (1<<TWINT) |(1 << TWSTA) | (1 << TWEN);

	loop_until_bit_is_set(TWCR, TWINT); // Vänta på att START condition har överförts

	if ((TWSR & 0b11111000) != TW_START)
		PORTB |= ERROR;//ERROR();	// Fel om TWI Statusregister olikt START
}


/************************************************************************/
/* Slave Transmitter Mode                                               */
/************************************************************************/

// Sätt bitarna i BITMASK explicit till BITS
#define SET_BITMASK(SFR, BITMASK, BITS) {SFR |= BITS; SFR &= ~((BITMASK) & ~(BITS)); }

#define WAIT_ON_BUS loop_until_bit_is_set(TWCR, TWINT)

void slave_transmit(Data data) {
	for (int i = 0; i < data.count; i++) {
		TWDR = data.data[i];

		if (i < data.count - 1) { // Inte sista byten
			SET_BITMASK(TWCR,
			    (1<<TWSTO) | (1<<TWINT) | (1<<TWEA),
			                 (1<<TWINT) | (1<<TWEA));

			WAIT_ON_BUS; // Vänta på att mastern fått meddelandet
		} else {
			SET_BITMASK(TWCR,
			    (1<<TWSTO) | (1<<TWINT) | (1<<TWEA),
			                 (1<<TWINT)            );
			WAIT_ON_BUS;
		}

		if ((TWSR & 0b11111000) != TW_ST_DATA_ACK)
			PORTB |= ERROR;
	}

	// Fyll ut med ettor ifall master vill ha mer data
	while((TWSR & 0b11111000) == TW_ST_DATA_ACK) {
		PORTB |= ERROR;
		TWDR = 0xFF;
		SET_BITMASK(TWCR,
		    (1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
		                             (1<<TWINT) | (1<<TWEA));
		WAIT_ON_BUS;
	}
}
