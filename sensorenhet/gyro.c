/*
 * gyro.c
 *
 * Created: 11/24/2014 10:25:55 AM
 *  Author: emiva760, carth567
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#define NSS_Low PORTB &= ~(1<<4)
#define NSS_High PORTB |= (1<<4)

#define WAIT_ON_EXCHANGE loop_until_bit_is_set(SPSR, SPIF)
#define ADCC_EN	 0b10010100
#define ADCC_DIS 0b10010000
#define ADCR	 0b10000000

void gyro_init(void) {
	DDRB |= (1<<DDB4) | (1<<DDB5) | (1<<DDB7); // Sätt NSS, MOSI och SCK som outputs

	//SPSR = (1<<SPI2X); /* Set SPI double frequency */

	SPCR = /* Configure SPI mode: */
		// (1<<SPIE) | /* should be activated to enable interruption from the SPI */
		(1<<SPE) | /* to enable SPI */
		(1<<MSTR) | /* to set Master SPI mode */
		(1<<CPOL) | /* SCK is high when idle */
		(1<<CPHA) | /* data is sampled on the trailing edge of the SCK */
		 //(1<<SPR1) | /* In this example SPI0=1, SPR1=0 (commented) and SPI2X=1 */
		(1<<SPR0); /* It sets SCK freq. in 8 times less than a system clock */


	// Starta timer 1 med 0 prescale och Clear Timer Compare-mode
	TCCR1B = (1<<CS10) | (1<<WGM12);

	// (1/target freq)/(1/cpu freq) - 1
	// (1/1000)/(1/8000000) - 1
	OCR1A = 7999;

	TIMSK1 = (1<<OCIE1A);

} /* DORD=0: the MSB is transmitted first */


/*
	Starta överföring
*/
uint8_t SPI_EXCH (uint8_t output) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		SPDR = output; /* Start transmission */

		/* Wait till a transmission and reception are completed */
		//while(!(SPSR & (1<<SPIF)));
		WAIT_ON_EXCHANGE;
	}
	return SPDR; /* Return Data Register */
}

/*
	Sätt aktiv
*/
void activate_adc(void) {
	NSS_Low;	// Sätt SS låg för att påbörja överföring
	SPI_EXCH(ADCC_EN);		// Skicka "starta AD-omvandling"
	NSS_High;	// Avsluta överföring
}


/*
	Sätt inaktiv
*/
void deactivate_adc(void) {
	NSS_Low;
	SPI_EXCH(ADCC_DIS);		//Skicka "adc sleep"
	NSS_High;
}


/*
  Läs vinkelfrekvensdata
*/
uint16_t read_ar_data(void) {
	uint8_t high_byte = 0, low_byte = 0;

	NSS_Low;
	SPI_EXCH(ADCR);					// Påbörja överföring
	high_byte = SPI_EXCH(0x00);		// Hämta MSBs
	low_byte = SPI_EXCH(0x00);		// Hämta LSBs
	NSS_High;
	uint16_t result = ((((high_byte & 0b00001111) << 8) | low_byte) >> 1);	// Skifta bort LSB?
	//return ((high_byte & 0b00001111) << 8) | low_byte;
	return result;
}

/* Konvertera till vinkel/sekund */
int ar_degrees(uint16_t adc_code)
{
	volatile int conversion = (adc_code * 25/12) + 400;
	volatile int offset = 2508;
	volatile double gain = 6.67; //double gain = 6.67;
	return (conversion - offset) / gain;	// 70ish = 90 Grader
	//return conversion;
}

int16_t fetch_angular_rate(void) {
	activate_adc();
	_delay_us(250);
	return ar_degrees(read_ar_data());
}

double current_degree = 0;
#define  DATA_POINTS 40
int16_t arates[DATA_POINTS];
int16_t current_arate;

int16_t arate_sum() {
	int16_t  sum = 0;
	for (int i = 0; i < DATA_POINTS; i++) {
		sum += arates[i];
	}
	return sum;
}

// Körs varje millisekund
ISR(TIMER1_COMPA_vect) {
	arates[current_arate] = fetch_angular_rate();
	current_arate = (current_arate + 1) % DATA_POINTS;

	int16_t arate_mean = arate_sum()/DATA_POINTS;
	current_degree += arate_mean / 1000.0; // 1000 som i 1 millisekund
	current_degree = current_degree;
}


double current_degrees() {
	return current_degree;
}

int16_t get_current_arate() {
	return arate_sum()/DATA_POINTS;
}
