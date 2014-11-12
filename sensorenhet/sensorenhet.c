/*
 * sensorenhet.c
 *
 * Created: 11/11/2014 2:06:50 AM
 *  Author: geoza435, dansj232
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#define IR_COUNT 5

/*
  Samma ordning som i sensors-taballen, vilket gör att man kan skriva
  t.ex. sensors[IR_FR]
*/
#define IR_FR ADC0D // = 0
#define IR_BR ADC1D // = 1
#define IR_FL ADC2D // = 2
#define IR_F  ADC3D // = 3
#define IR_BL ADC4D // = 4

unsigned int current_sensor = 0;
char sensors[IR_COUNT] = {IR_FR, IR_BR, IR_FL, IR_F, IR_BL};
// TODO: volatile bara för debug
volatile uint16_t x[IR_COUNT];
volatile double mm[IR_COUNT];

//Interruptrutin för AD omvandlare
ISR (ADC_vect) {
	/*
	  The ADC generates a 10-bit result which is presented in the ADC
	  Data Registers, ADCH and ADCL. By default, the result is
	  presented right adjusted, but can optionally be presented left
	  adjusted by setting the ADLAR bit in ADMUX.

	  If the result is left adjusted and no more than 8-bit precision
	  is required, it is sufficient to read ADCH. Otherwise, ADCL must
	  be read first, then ADCH, to ensure that the content of the Data
	  Registers belongs to the same conversion.
	*/
	x[current_sensor] = ADCL | (ADCH<<8);
	
	mm[current_sensor] = -(4.95383020937e-11)*pow(x[current_sensor],5)
		+ (1.08781340946e-07)*pow(x[current_sensor],4)
		- (9.20458860953e-05)*pow(x[current_sensor],3)
		+  0.0379590492055*pow(x[current_sensor],2)
		-  7.97634806239*x[current_sensor]
		+  800.371349512;
	
	current_sensor = (current_sensor + 1) % IR_COUNT;
	
	/*
	  Optionally, AVCC or an internal 2.56V reference voltage may be
	  connected to the AREF pin by writing to the REFSn bits in the
	  ADMUX Register.

	  The analog input channel and differential gain are selected by
	  writing to the MUX bits in ADMUX.
	*/
	// ADMUX: p. 256... Table 21-4.
	// REFS är default 00
	// ? ? ? ? ? ? ? ?
	// 0 0 0 X X X X X
	//----------------
	// ? ? ? X X X X X
	ADMUX |= sensors[current_sensor];                  // Sätt ettor
	ADMUX &= ~(0b00011111 & ~sensors[current_sensor]); // Sätt nollor
	
	/*
	  A single conversion is started by writing a logical one to the
	  ADC Start Conversion bit, ADSC. This bit stays high as long as
	  the conversion is in progress and will be cleared by hardware
	  when the conversion is completed.
	*/
	//        Start conversion
	ADCSRA |= (1<<ADSC);
}

/*
  p.241 ...

  The minimum value represents GND and the maximum value represents
  the voltage on the AREF pin minus 1 LSB.

  p.242 ...

  If a different data channel is selected while a conversion is in
  progress, the ADC will finish the current conversion before
  performing the channel change.
*/

int main(void) {
	/* p.241..
	   The ADC is enabled by setting the ADC Enable bit, ADEN in
	   ADCSRA. Voltage reference and input channel selections will not
	   go into effect until ADEN is set.
	*/
	
	sei(); //Aktivera interrupts
	// Börja med första sensorn
	ADMUX |= sensors[current_sensor];
	ADMUX &= ~(0b00011111 & ~sensors[current_sensor]);
	
	//        AD Enable   Interrupt enable   AD Start conversion
	ADCSRA |= (1<<ADEN) | (1<<ADIE)        | (1<<ADSC);

	while(1) {}
}
