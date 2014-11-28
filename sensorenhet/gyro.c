/*
 * gyro.c
 *
 * Created: 11/24/2014 10:25:55 AM
 *  Author: emiva760, carth567
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "gyro.h"

#define F_CPU 8000000UL
#include <util/delay.h>

#define NSS_Low PORTB &= ~(1<<4)
#define NSS_High PORTB |= (1<<4)

#define WAIT_ON_EXCHANGE loop_until_bit_is_set(SPSR, SPIF)
#define ADCC_EN	 0b10010100
#define ADCC_DIS 0b10010000
#define ADCR	 0b10000000

// SPI_Init
void SPI_Init(void) {
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
} /* DORD=0: the MSB is transmitted first */


/*
	Starta överföring
*/
uint8_t SPI_EXCH (uint8_t output) {
	SPDR = output; /* Start transmission */

	/* Wait till a transmission and reception are completed */
	//while(!(SPSR & (1<<SPIF)));
	WAIT_ON_EXCHANGE;

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
	int conversion = (adc_code * 25/12) + 400;
	int offset = 2500;
	double gain = 13.33; //double gain = 6.67;
	return (conversion - offset) / gain;	// 70ish = 90 Grader
	//return conversion;
}

int fetch_angular_rate(void) {
	activate_adc();
	_delay_us(250);
	return ar_degrees(read_ar_data());
}


int main(void) {
	sei();
	SPI_Init();
	//activate_adc();

	volatile uint16_t test = 0;
	volatile int test2 = 0;
	double degree = 0;
	volatile double degree2 = 0;
	uint16_t count = 0;
	//test = read_ar_data();

	while (1)
	{
		test2 = fetch_angular_rate();
		degree += test2 / 1000.0;
		
		if (count++ > 5000)
		{
			degree2 = degree;
			count = 0;
		}
		/*
		activate_adc();
		_delay_ms(1);
		test = read_ar_data();
		test2 = ar_degrees(test);
		degree += test2 / 1000.0;
		if (count++ > 5000)
		{
			degree2 = degree;
			count = 0;
		}
		*/
	}

	//	Sätt bra output-portar för debuggning med logikanalysator här

	return 0;
}
