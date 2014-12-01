/*
 * beslutsenhet.c
 *
 * Created: 2014-11-13 14:10:27
 *  Author: erima694 & eribo740
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <common.h>
#include <modulkom.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

//Set CPU clock
#define F_CPU 8000000UL
#include <util/delay.h>

volatile uint8_t autoMode;
volatile uint8_t makeDecisionFlag;
volatile uint8_t pdFlag;
volatile uint8_t turn;
volatile uint8_t send_stop = false;

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
Bus_data master_data_to_send = {0};
Bus_data master_data_to_receive = {0};

void send_move_data(double forward, double side, double turn) {
	Move_data move_data;
	move_data.count = command_lengths[MOVE]+2;
	move_data.id = MOVE;
	move_data.forward_speed = forward;
	move_data.side_speed = side;
	move_data.turn_speed = turn;
	send_data(which_device[MOVE], move_data.bus_data);
}

Sensor_data getSensorData() {
	volatile Sensor_data sensor_data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		master_data_to_receive.count = command_lengths[SENSOR_DATA]+2;
		fetch_data(SENSOR, &master_data_to_receive);
		sensor_data = (Sensor_data)master_data_to_receive;
	}
	return sensor_data;
}

void waitForCorrectValues()
{
	volatile Sensor_data sensor_data;
	do
	{
		sensor_data = getSensorData();
		_delay_ms(20);
		
	} while (sensor_data.bl + sensor_data.br > 75);
}

double MAX_ADJUSTMENT = 0.5; //Constant to stop the robot from turning like crazy
double P = 0.01; //Constant for the proportional part
double D = 0.1; //Constant for the derivative part

#define ERROR_COUNT 10
double errors[ERROR_COUNT] = {0};
uint8_t current_error = 0; // Nuvarande error i errors-arrayen
double prevError = 0; //The previous error

double error_mean() {
	double sum = 0;
	for (int i = 0; i < ERROR_COUNT; i++) {
		sum += errors[i];
	}
	return sum/ERROR_COUNT;
}

void nextError() {
	current_error = (current_error + 1) % ERROR_COUNT;
}

void pdAlgoritm(double distanceRight, double distanceLeft) {
	errors[current_error] = distanceRight - distanceLeft;
	nextError();

	double error = error_mean();

	double d_adjustment = 0;
	double p_adjustment = 0;
	double adjustment = 0;

	d_adjustment = D*(error - prevError);
	p_adjustment = P*error;

	adjustment = p_adjustment + d_adjustment;
	adjustment = fmax(fmin(adjustment, MAX_ADJUSTMENT), -MAX_ADJUSTMENT);

	prevError = error;

	PD_Data pd_data = {
		.id = PD_DATA,
		.count = command_lengths[PD_DATA] + 2,
		.error = error,
		.p = p_adjustment,
		.d = d_adjustment,
		.adjustment = adjustment
	};

	send_data(COMMUNICATION, pd_data.bus_data);

	send_move_data(0.5, 0, adjustment);
}

void waitForGyro(double deg) {
	//double startGyro = getSensorData().gyro;
	//volatile Sensor_data sensor_data;
	//do
	//{
		//_delay_ms(20);
		//sensor_data = getSensorData();
		//
	//} while (fabs(sensor_data.gyro - startGyro) < deg);
	_delay_ms(2200);
}

void makeDecision(void) {
	volatile Sensor_data sensor_data = getSensorData();
	
	//if(sensor_data.f < 30)
	//{
		//send_move_data(0, 0, 0);//stop
		//TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
		//TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
		//pdFlag  = 0;
		//makeDecisionFlag = 0;
	//}
	
	if(sensor_data.fr<150) {
		if(sensor_data.fl<150) {

			//send_move_data(0.5, 0, 0);//go forward
			
		}else {
			if(sensor_data.f<30) {
				send_move_data(0, 0, -0.8);//turn left
				waitForGyro(90);//Wait for 90 degree turn, by asking gyro
				send_move_data(0.5, 0, 0);//go forward
				waitForCorrectValues();
			}else {
				if(sensor_data.fr<80) {
					_delay_ms(500);
					send_move_data(0.5, 0, -0.8);//turn left
					waitForGyro(90);//Wait for 90 degree turn, by asking gyro
					send_move_data(0.5, 0, 0);//go forward
					waitForCorrectValues();
				}
			}
		}
	}else {
		if(sensor_data.f<30) {
			send_move_data(0, 0, 0.8);//turn right
			waitForGyro(90);//Wait for 90 degree turn, by asking gyro
			send_move_data(0.5, 0, 0);//go forward
			waitForCorrectValues();
		}else {
			if(sensor_data.fl<80) {
				_delay_ms(500);
				send_move_data(0.5, 0, 0.8);//turn right
				waitForGyro(90);//Wait for 90 degree turn, by asking gyro
				send_move_data(0.5, 0, 0);//go forward
				waitForCorrectValues();
			}else {
				Bus_data stop;
				stop.id = STOP_TIMER;
				stop.count =  command_lengths[STOP_TIMER];
				send_data(which_device[STOP_TIMER], stop);//celebrate
				send_move_data(0, 0, 0);//stop
				
				autoMode = 0;
				TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
				TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
			}
		}
	}
}

ISR(TIMER1_OVF_vect) {
	makeDecisionFlag = 1;
	TCNT1H = 0xB0; //Reset Timer1 high register
	TCNT1L = 0x00; //Reset Timer1 low register
}

ISR(TIMER3_OVF_vect) {
	pdFlag = 1;
	TCNT3H = 0xFF-(deltaT >> 8); //Reset Timer3 high register
	TCNT3L = 0xFF-(deltaT & 0x00FF); //Reset Timer3 low register
}

//Initialize the timer interrupt to happen
//approximately once per second 
void initTimer(void) {
	//TIMSK1 = (1<<TOIE1);//Enable timer overflow interrupt for Timer1
	TCNT1H = 0x80; //Initialize Timer1 high register
	TCNT1L = 0x00; //Initialize Timer1 low register
	TCCR1B = (1<<CS11)|(1<<CS10);//Use clock/64 prescaler
	
	//TIMSK3 = (1<<TOIE3);//Enable timer overflow interrupt for Timer3
	TCNT3H = 0x00; //Initialize Timer3 high register
	TCNT3L = 0x00; //Initialize Timer3 low register
	TCCR3B = (1<<CS31)|(1<<CS30);//Use clock/64 prescaler
}

Bus_data prepare_data() {
	return data_to_send;
}

void interpret_data(Bus_data data){
	data_to_receive = data;
	
	if(data_to_receive.id == SET_P) {
		
		Constant_data constant_data = (Constant_data)data_to_receive;
		P = constant_data.constant;
		
	}else if(data_to_receive.id == SET_D) {
		
		Constant_data constant_data = (Constant_data)data_to_receive;
		D = constant_data.constant;
		
	}else if(data_to_receive.id == CHANGEMODE) {
		
		autoMode = data_to_receive.data[0];
		if(autoMode == 1) {
			TIMSK1 |= (1<<TOIE1);//Enable timer overflow interrupt for Timer1
			TIMSK3 |= (1<<TOIE3);//Enable timer overflow interrupt for Timer3
			send_stop = false;
		}else {
			TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
			TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
			send_stop = true;
		}
		
	}
}

int main(void) {
	set_as_slave(F_CPU, prepare_data, interpret_data, DECISION);
	set_as_master(F_CPU);
	
	P = 1.0/40.0;
	D = (1.0/5.0)*(double)0x4FFF;
	makeDecisionFlag = 0;
	pdFlag = 0;
	autoMode = 0;
	turn = 1;
	stableValues = 2;
	
	initTimer();
	sei();
    while(1) {
		
        if(makeDecisionFlag == 1) {
			makeDecision();
			makeDecisionFlag = 0;
		}
		
		_delay_ms(20);
		
		if(pdFlag == 1) {
			volatile Sensor_data sensor_data = getSensorData();
			pdAlgoritm(sensor_data.br, sensor_data.bl);
			pdFlag = 0;
		}
		
		if(send_stop) {
			send_move_data(0, 0, 0);
		}

		_delay_ms(20);
    }
}
