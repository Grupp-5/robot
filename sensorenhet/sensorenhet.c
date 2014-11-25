/*
 * sensorenhet.c
 *
 * Created: 11/11/2014 2:06:50 AM
 *  Author: geoza435, dansj232
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <math.h>
#include <common.h>
#include <modulkom.h>

#define IR_COUNT 5

/*
  Samma ordning som i sensors-taballen, vilket gör att man kan skriva
  t.ex. sensors[IR_FR]
*/
#define IR_FR ADC0D // = 0 (20 - 150 cm)
#define IR_BR ADC1D // = 1 (10 -  80 cm)
#define IR_FL ADC2D // = 2 (20 - 150 cm)
#define IR_F  ADC3D // = 3 (4  -  30 cm)
#define IR_BL ADC4D // = 4 (10 -  80 cm)

unsigned int current_sensor_g = 0;
char sensors[IR_COUNT] = {IR_FR, IR_BR, IR_FL, IR_F, IR_BL};
// TODO: volatile bara för debug
volatile uint16_t x_g[IR_COUNT];
volatile double cm[IR_COUNT];

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};

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
	x_g[current_sensor_g] = ADCL | (ADCH<<8);
	
	current_sensor_g = (current_sensor_g + 1) % IR_COUNT;
	
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
	ADMUX |= sensors[current_sensor_g];                  // Sätt ettor
	ADMUX &= ~(0b00011111 & ~sensors[current_sensor_g]); // Sätt nollor
	
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

#define AD_PRESCALE_2   0b000
#define AD_PRESCALE_2_  0b001
#define AD_PRESCALE_4   0b010
#define AD_PRESCALE_8   0b011
#define AD_PRESCALE_16  0b100
#define AD_PRESCALE_32  0b101
#define AD_PRESCALE_64  0b110
#define AD_PRESCALE_128 0b111

Bus_data prepare_data() {
	Sensor_data sensor_data;
	sensor_data.count = command_lengths[SENSOR_DATA] + 2;
	sensor_data.id = SENSOR_DATA;
	sensor_data.fr = cm[IR_FR];
	sensor_data.br = cm[IR_BR];
	sensor_data.fl = cm[IR_FL];
	sensor_data.f = cm[IR_F];
	sensor_data.bl = cm[IR_BL];
	data_to_send = sensor_data.bus_data;
	return data_to_send;
}

void interpret_data(Bus_data data){
	data_to_receive = data;
}

int main(void) {
	/* p.241..
	   The ADC is enabled by setting the ADC Enable bit, ADEN in
	   ADCSRA. Voltage reference and input channel selections will not
	   go into effect until ADEN is set.
	*/
	set_as_slave(prepare_data, interpret_data, SENSOR);
	sei(); //Aktivera interrupts
	// Börja med första sensorn
	ADMUX |= sensors[current_sensor_g];
	ADMUX &= ~(0b00011111 & ~sensors[current_sensor_g]);
	
	//        AD Enable   Interrupt enable   AD Start conversion  Sista 3 bitarna
	ADCSRA |= (1<<ADEN) | (1<<ADIE)        | (1<<ADSC)          | AD_PRESCALE_32;

	volatile uint8_t current_sensor = 0;
	double x[IR_COUNT] = {0};
	while(1) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			x[current_sensor] = x_g[current_sensor];
		}
		switch(current_sensor) {
		case IR_F:
			cm[current_sensor] = 1.26464293347e-14*pow(x[current_sensor], 6)
			+ -3.24803038942e-11*pow(x[current_sensor], 5)
			+ 3.36401952493e-08*pow(x[current_sensor], 4)
			+ -1.81285077491e-05*pow(x[current_sensor], 3)
			+ 0.00548098207662*pow(x[current_sensor], 2)
			+ -0.934445496366*pow(x[current_sensor], 1)
			+ 83.3762148745*pow(x[current_sensor], 0);
			break;
		case IR_FL:
			case IR_FR:
			cm[current_sensor] = 1.5358134636e-19*pow(x[current_sensor], 8)
			+ -5.64979179032e-16*pow(x[current_sensor], 7)
			+ 8.97290364989e-13*pow(x[current_sensor], 6)
			+ -8.09866506873e-10*pow(x[current_sensor], 5)
			+ 4.58857846301e-07*pow(x[current_sensor], 4)
			+ -0.000169401981795*pow(x[current_sensor], 3)
			+ 0.0406130112214*pow(x[current_sensor], 2)
			+ -6.02157204247*x[current_sensor]
			+ 485.450912561;
			break;
		case IR_BR:
		case IR_BL:
			cm[current_sensor] = 4.61115274448e-09*pow(x[current_sensor], 4)
			+ -7.53561554655e-06*pow(x[current_sensor], 3)
			+ 0.00454026336693*pow(x[current_sensor], 2)
			+ -1.24981836081*pow(x[current_sensor], 1)
			+ 154.383035165*pow(x[current_sensor], 0);
			break;
			default:
			cm[current_sensor] = 0xFE;
		}
		current_sensor = (current_sensor + 1) % IR_COUNT;
	}
}
