/*
 * sensorenhet.c
 *
 * Created: 11/11/2014 2:06:50 AM
 *  Author: geoza435
 */


#include <avr/io.h>

#include <math.h>

#define IR_F  ADC3D // = 3
#define IR_FR ADC0D // = 0
#define IR_FL ADC2D // = 2
#define IR_BR ADC1D // = 1
#define IR_BL ADC4D // = 4

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
	// Init ADC
	/*
	  Optionally, AVCC or an internal 2.56V reference voltage may be
	  connected to the AREF pin by writing to the REFSn bits in the
	  ADMUX Register.
	  
	  The analog input channel and differential gain are selected by
	  writing to the MUX bits in ADMUX.
	*/
	// ? ? ? ? ? ? ? ?
	// 0 0 0 X X X X X
	//------------------
	// ? ? ? X X X X X
	// ADMUX: p. 256... Table 21-4.
	ADMUX |= IR_BL;                 // Sätt ettor
	ADMUX &= ~(0b00011111 & ~IR_F); // Sätt nollor
	/* p.241..
	
	   The ADC is enabled by setting the ADC Enable bit, ADEN in
	   ADCSRA. Voltage reference and input channel selections will not
	   go into effect until ADEN is set.
	*/
	// p. 257
	// REFS är default 00
	//        AD Enable
	ADCSRA |= (1<<ADEN);
	
	volatile uint8_t low_result = 0;
	volatile uint8_t high_result = 0;
	volatile uint16_t result = 0;
	
	volatile double cm = 0;
	
	while(1) {
		/*
		  A single conversion is started by writing a logical one to
		  the ADC Start Conversion bit, ADSC. This bit stays high as
		  long as the conversion is in progress and will be cleared by
		  hardware when the conversion is completed.
		*/
		//        Start conversion
		ADCSRA |= (1<<ADSC);
		loop_until_bit_is_clear(ADCSRA, ADSC);
		
		/*
		  The ADC generates a 10-bit result which is presented in the
		  ADC Data Registers, ADCH and ADCL. By default, the result is
		  presented right adjusted, but can optionally be presented
		  left adjusted by setting the ADLAR bit in ADMUX.
		  
		  If the result is left adjusted and no more than 8-bit
		  precision is required, it is sufficient to read ADCH.
		  Otherwise, ADCL must be read first, then ADCH, to ensure
		  that the content of the Data Registers belongs to the same
		  conversion.
		*/
		result = ADCL | (ADCH<<8);
		
		//e^(-0.00699137730132836*y + 3.94779771615008) + 3.3
		//cm = exp(-0.006991377301*result + 3.947797) + 3.3;
		//e^(-0.00661346501477007*y + 3.94779771615008) + 3.3
		cm = exp(-0.00661346501477007*result + 3.94779771615008) + 3.3;
	}
}
