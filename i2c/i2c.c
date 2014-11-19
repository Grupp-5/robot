/*
 * i2c.c
 *
 * Created: 11/14/2014 12:05:29 PM
 *  Author: emiva760, geoza435, carth567
 */

#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "i2c.h"

#define F_CPU 8000000UL
#include <util/delay.h>

#define ERROR 1

// Sätt bitarna i SFR med BITMASK explicit till BITS
/*
 *     SFR: 0b????????
 * BITMASK: 0b00010101
 *    BITS: 0b00000101
 *          ---------->
 *     SFR: 0b???0?1?1
 */
#define SET_BITMASK(SFR, BITMASK, BITS) SFR = (BITS) | (~(BITMASK) & (SFR))

#define WAIT_ON_BUS loop_until_bit_is_set(TWCR, TWINT)

// De viktiga bitarna i TWSR
#define TWSR_BITS (TWSR & 0b11111000)

// Spammar pin BIT på port SFR COUNT gånger
#define SPAM(SFR, BIT, COUNT)                                  \
    for (int i = 0; i < COUNT; i++) {                          \
        SFR |= _BV(BIT);                                       \
        _delay_us(20);                                         \
        SFR &= 0xFF & ~_BV(BIT);                               \
        _delay_us(20);                                         \
    }

// Globala funktionspekare som bestämmer hur en slave ska agera när
// den får ett READ eller WRITE-kommando.
Data (*prepare_data_func)();
void (*interpret_data_func)(Data);

void send_stop_condition(void) {
	PORTB |= (1<<PORTB7);

	// TWIE alltid 1 för att det ska inte spela någon större roll då den sätts
	// till 0 i send_start_condition.
	// TODO: TWEA (kanske, men verkar inte) bero på om det är en master+slave
	SET_BITMASK(TWCR,
		(1<<TWSTA) | (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA),
		             (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA));
}

void master_init(uint32_t f_cpu, uint32_t bitrate) {
	/* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
	TWSR = 0;                       /* no prescaler */
	TWBR = ((f_cpu/bitrate)-16)/2;  /* must be > 10 for stable operation */

	SET_BITMASK(TWCR,(1<<TWEN),
	                 (1<<TWEN));
}

void slave_init(Data(*prepare_data)(), void(*interpret_data)(Data), uint8_t slave_address) {
	// Ställer in de globala funktionspekarna
	prepare_data_func = prepare_data;
	interpret_data_func = interpret_data;

	TWAR = (slave_address << 1);
	TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
}

void send_start_condition(void) {
	// Skicka startvillkor
	SET_BITMASK(TWCR,
		(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA),
		(1<<TWSTA) |             (1<<TWINT) | (1<<TWEN)                        );

	WAIT_ON_BUS; // Vänta på att START condition har överförts

	// Fel om TWI Statusregister olikt START eller Repeated start
	if (TWSR_BITS != TW_START && TWSR_BITS != TW_REP_START) {
		PORTB = ~PORTB;
		_delay_ms(5);
		PORTB = ~PORTB;
		PORTB |= (1<<PORTB3);
	}
}



/************************************************************************/
/* Master Receiver Mode                                                 */
/************************************************************************/

/* Hämta från slav */
void master_receive(uint8_t slave_address, Data *data) {
	cli();
	send_start_condition();

	TWDR = (slave_address << 1) | TW_READ;	// Ladda SLA+R

	// Rensa TWINT-bit för att påbörja överföring av adress
	SET_BITMASK(TWCR,
		(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEN),
		                         (1<<TWINT) | (1<<TWEN));

	WAIT_ON_BUS; // Vänta på att sla_r har överförts och (N)ACK har mottagits

	if (TWSR_BITS == TW_MR_ARB_LOST) { // "arbitration lost in SLA+R or NACK"
		PORTB |= (1<<PORTB1);
		return; // TODO: Gör rätt sak
	}

	if(TWSR_BITS == TW_MR_SLA_NACK) {
		send_stop_condition();
		SPAM(PORTB, PORTB3, 11);
		// TODO: Signalera att något blev fel till anroparen
		sei();
		return;
	}

	if (TWSR_BITS != TW_MR_SLA_ACK) { // Något annat än ACK mottaget hände
		PORTB |= (1<<PORTB0);
		return; // TODO: Gör rätt sak
	}

	//TODO: Kolla efter TW_MR_ARB_LOST och TW_MR_SLA_NACK (p.223)

	// Hämta data
	for (int i = 0; i < data->count; i++) {
		if (i < data->count -1) { // Inte sista datat
			SET_BITMASK(TWCR,
				(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
				                         (1<<TWINT) | (1<<TWEA));
		} else {                  // Sista datat
			SET_BITMASK(TWCR,
				(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
				                         (1<<TWINT)            );
		}

		WAIT_ON_BUS; // Vänta på (N)ACK-bit skickats,
					 // dvs att slaven skickat datat

		if (TWSR_BITS == TW_MR_ARB_LOST) { // "arbitration lost in SLA+R or NACK"
			SPAM(PORTB, PORTB4, 200);
			return; // TODO: Gör rätt sak
		}

		data->data[i] = TWDR; // Spara mottagen data
	}

	if (TWSR_BITS != TW_MR_DATA_NACK) {
		PORTB |= (1<<PORTB2); // Kolla att överföring har lyckats
	}

	send_stop_condition();
	sei();
}

/************************************************************************/
/* Master Transmitter Mode                                              */
/************************************************************************/
void master_transmit(uint8_t slave_address, Data data) {
	cli();
	send_start_condition();

	TWDR = (slave_address << 1) | TW_WRITE;	// Ladda SLA+W

	// Rensa TWINT-bit för att påbörja överföring av adress
	TWCR = (1 << TWINT) | (1 << TWEN);

	WAIT_ON_BUS; // Vänta på att sla_w har överförts och (N)ACK har mottagits

	if (TWSR_BITS == TW_MT_ARB_LOST) { // Arbitration lost
		// Gå in i interrupt-styrd slave-mode
		// TODO: Vet fortfarande inte riktigt vad som ska hända här.
		// Får ingen trigg på det här..
		SPAM(PORTB, PORTB4, 300);
	}

	if (TWSR_BITS == TW_MT_SLA_NACK) {
		// TODO: sätt error att överföringen misslyckades
		send_stop_condition();
		SPAM(PORTB, PORTB1, 21);
		sei();
		return;
	}

	if (TWSR_BITS != TW_MT_SLA_ACK) { // Något annat än ACK mottaget hände
		//PORTB |= (1<<PORTB0);
		return;
	}

	for (int i = 0; i < data.count; i++) {
		TWDR = data.data[i];

		if (i < data.count - 1) { // Inte sista byten
			SET_BITMASK(TWCR,
				(1<<TWSTO) | (1<<TWINT) | (1<<TWEA),
							 (1<<TWINT) | (1<<TWEA));
			WAIT_ON_BUS; // Vänta på att slaven fått meddelandet
		} else {
			SET_BITMASK(TWCR,
				(1<<TWSTO) | (1<<TWINT) | (1<<TWEA),
							 (1<<TWINT)            );
			WAIT_ON_BUS;
		}
	}

	if (TWSR_BITS != TW_MT_DATA_ACK) {
		PORTB |= (1<<PORTB2); // Kolla att överföring har lyckats
	}

	send_stop_condition();
	sei();
}

/************************************************************************/
/* Slave Transmitter Mode                                               */
/************************************************************************/

void slave_transmit(Data data) {
	for (int i = 0; i < data.count; i++) {
		TWDR = data.data[i];

		if (i < data.count - 1) { // Inte sista byten
			SET_BITMASK(TWCR,
			    (1<<TWSTO) | (1<<TWINT) | (1<<TWEA),
			                 (1<<TWINT) | (1<<TWEA));
			WAIT_ON_BUS; // Vänta på att mastern fått meddelandet

			// Förväntade sig en ACK men fick en NACK
			if (TWSR_BITS == TW_ST_DATA_NACK) {
				PORTB |= (1<<PORTB1);
				break;
			}
		} else {
			SET_BITMASK(TWCR,
			    (1<<TWSTO) | (1<<TWINT) | (1<<TWEA),
			                 (1<<TWINT)            );
			WAIT_ON_BUS;
		}
	}

	// TODO: Kanske göra något? Förmodligen inte.
	if (TWSR_BITS != TW_ST_DATA_NACK)
		PORTB |= (1<<PORTB1);

	if (TWSR_BITS == TW_ST_DATA_ACK) {
		//PORTB |= (1<<PORTB2);
	}

	SET_BITMASK(TWCR,
		(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
		                         (1<<TWINT) | (1<<TWEA));
}

/************************************************************************/
/* Slave Receiver Mode                                                  */
/************************************************************************/
Data slave_receive() {
	volatile Data data = {0};
	// Hämta så mycket data som går
	while (1) {
		if (data.count >= MAX_DATA) {
			SET_BITMASK(TWCR,
				(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
				                         (1<<TWINT)            );
			break;
		}
		SET_BITMASK(TWCR,
			(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
			                         (1<<TWINT) | (1<<TWEA));

		WAIT_ON_BUS; // Vänta på ACK-bit skickats,
		             // dvs att mastern skickat datat
		if (TWSR_BITS == TW_SR_STOP) {
			break;
		}

		data.data[data.count++] = TWDR; // Spara mottagen data
	}

	if (TWSR_BITS != TW_SR_STOP) { // Konstig check
		PORTB |= (1<<PORTB1); // Kolla att överföring har lyckats
	}

	SET_BITMASK(TWCR,
		(1<<TWSTA) | (1<<TWSTO)| (1<<TWINT) | (1<<TWEA),
								 (1<<TWINT) | (1<<TWEA));
	return data;
}

ISR(TWI_vect)
{
	cli();
	PORTB |= (1<<PORTB0);
	switch(TWSR_BITS) {
		case REQUEST_TO_WRITE:
			slave_transmit(prepare_data_func());
			break;
		case REQUEST_TO_READ:
			interpret_data_func(slave_receive());
			break;
		default:
			PORTB |= (1<<PORTB3);
	}
	_delay_us(10);
	PORTB = 0;
	sei();
}
