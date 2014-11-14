/*
 * beslutsenhet.c
 *
 * Created: 2014-11-13 14:10:27
 *  Author: erima694 & eribo740
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t MAX_ADJUSTMENT = 100; //Constant to stop the robot from turning like crazy
uint8_t P = 5; //Constant for the proportional part
uint8_t D = 4; //Constant for the derivative part
uint8_t prevError = 0; //The previous error

volatile uint8_t fleftSensorValue;
volatile uint8_t frightSensorValue;
volatile uint8_t fmidSensorValue;
volatile uint8_t autoMode;


int pdAlgoritm(uint8_t distanceRight, uint8_t distanceLeft) {
	uint8_t error = distanceRight - distanceLeft;
	uint8_t adjustment = P*error + D*(error - prevError);
	prevError = error;
	if(adjustment > MAX_ADJUSTMENT)
	{
		adjustment = MAX_ADJUSTMENT;
	}
	else if(adjustment < -MAX_ADJUSTMENT)
	{
		adjustment = -MAX_ADJUSTMENT;
	}
	return adjustment;
}

ISR(TIMER1_OVF_vect) {
	uint8_t left = 0;
	uint8_t right = 0;
	uint8_t adjustment = pdAlgoritm(right, left);
		
	TCNT1H = 0x80; //Reset Timer1 high register
	TCNT1L = 0x00; //Reset Timer1 low register
}

ISR(TIMER3_OVF_vect) {
	makeDecision();
	TCNT3H = 0x80; //Reset Timer3 high register
	TCNT3L = 0x00; //Reset Timer3 low register
}

//Initialize the timer interrupts to happen
//approximately once per second 
void initTimer(void) {
	TIMSK1 = (1<<TOIE1);//Enable timer overflow interrupt for Timer1
	TCNT1H = 0x80; //Initialize Timer1 high register
	TCNT1L = 0x00; //Initialize Timer1 low register
	TCCR1B = (1<<CS11)|(1<<CS10);//Use clock/64 prescaler
	
	TIMSK3 = (1<<TOIE3);//Enable timer overflow interrupt for Timer3
	TCNT3H = 0x00; //Initialize Timer3 high register
	TCNT3L = 0x00; //Initialize Timer3 low register
	TCCR3B = (1<<CS31)|(1<<CS30);//Use clock/64 prescaler
}

void getSensorValues(void) {
	fleftSensorValue = 0;
	frightSensorValue = 0;
	fmidSensorValue = 0;
}

void makeDecision(void) {
	getSensorValues();
	
	if(frightSensorValue<150) {
		if(fleftSensorValue<150) {
			//go forward
		}else {
			if(fmidSensorValue<30) {
				//turn 90 left
				//go forward
			}else {
				if(frightSensorValue<80) {
					//turn 90 lef
					//go forward
				}
			}
		}
	}else {
		if(fmidSensorValue<30) {
			//turn 90 right
			//go forward
		}else {
			if(fleftSensorValue<80) {
				//turn 90 right
				//go forward
			}else {
				//celebrate
			}	
		}
	}
}

int main(void) {
	autoMode = 0;
	initTimer();
	sei();
    while(1) {
        
    }
}