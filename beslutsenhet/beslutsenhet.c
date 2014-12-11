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
#include <math.h>

//Set CPU clock
#define F_CPU 8000000UL
#include <util/delay.h>

uint16_t deltaT = 0x6FFF;//Change this to change time between PD-adjustments
double MAX_ADJUSTMENT = 0.3; //Constant to stop the robot from turning like crazy
double TURN_MAX_ADJUSTMENT = 0.2; //Constant to stop the robot from turning like crazy
double P; //Constant for the proportional part
double D; //Constant for the derivative part
double prevError = 0; //The previous error

volatile uint8_t autoMode;
volatile uint8_t makeDecisionFlag;
volatile uint8_t pdFlag;
volatile uint8_t turn;
volatile uint8_t stableValues;

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
Bus_data master_data_to_send = {0};
Bus_data master_data_to_receive = {0};
	
typedef enum {
	LEFT,
	RIGHT,
} Direction;
	
void send_to_bus(Device_id dev_id, Data_id data_id, uint8_t arg_count, uint8_t data_array[]) {
	master_data_to_send.id = data_id;
	master_data_to_send.count = arg_count+2;
	for(int i = 0; i<arg_count; i++) {
		master_data_to_send.data[i] = data_array[i];
	}
	send_data(dev_id, master_data_to_send);
}

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
	master_data_to_receive.count = command_lengths[SENSOR_DATA]+2;
	fetch_data(SENSOR, &master_data_to_receive);
	sensor_data = (Sensor_data)master_data_to_receive;

	return sensor_data;
}

//Wait until the robot is in a corridor
//again after a turn
void waitForCorrectValues() {
	volatile Sensor_data sensor_data;
	do {
		sensor_data = getSensorData();
		_delay_ms(20);
		
	} while (sensor_data.bl + sensor_data.br > 75);
}

//Uses a combination of sidestepping and turning
//to avoid a change in the distance to the walls
//due to the rotation
void pdAlgoritm(double distanceRight, double distanceLeft) {
	double error = distanceRight - distanceLeft;
	double turn_adjustment = 0;
	double side_adjustment = 0;
	
	if(distanceRight < 70 && distanceLeft < 70) {
		if(turn == 1) {
			if(stableValues == 3) {
				turn_adjustment =  D*((error+prevError)/2 - prevError)/(double)deltaT;
			
				if(turn_adjustment > TURN_MAX_ADJUSTMENT) {
					turn_adjustment = TURN_MAX_ADJUSTMENT;
				} else if(turn_adjustment < -TURN_MAX_ADJUSTMENT) {
					turn_adjustment = -TURN_MAX_ADJUSTMENT;
				}
			
				if(fabs(turn_adjustment) < 0.01) {
					turn = 0;//The robot is walking straight, time to sidestepp
				}
			} else {
				stableValues++;//Wait until the derivative have stabilized
			}
		
		} else {
			side_adjustment = P*error;
			if(side_adjustment > MAX_ADJUSTMENT) {
				side_adjustment = MAX_ADJUSTMENT;
			} else if(side_adjustment < -MAX_ADJUSTMENT) {
				side_adjustment = -MAX_ADJUSTMENT;
			}
		
			if(fabs(side_adjustment) < 0.18) {
				turn = 1;//The robot is in the middle of the corridor, time to straighten it up
				stableValues = 0;//The robot needs to wait until the derivative have stabilized 
			}
		}
	
		PD_Data pd_data = {
			.id = PD_DATA,
			.count = command_lengths[PD_DATA] + 2,
			.error = error,
			.p = side_adjustment,
			.d = turn_adjustment
		};
	
		send_data(COMMUNICATION, pd_data.bus_data);//Send PD-values to the computer were they can be analyzed
	
		prevError = (error+prevError)/2;//Removes some of the peaks in the derivative
	
		send_move_data(0.5, side_adjustment, turn_adjustment);
	}else {
		send_move_data(0.5, 0, 0);//Go Forward
	}
}

//Wait until the robot has turned deg
//degrees
void waitForGyro(double deg) {
	double startGyro = getSensorData().gyro;
	volatile Sensor_data sensor_data;
	do {
		_delay_ms(20);
		sensor_data = getSensorData();
		
	} while (fabs(sensor_data.gyro - startGyro) < deg);
}

//Wait until the robots back sensor in the direction dir
//also has arrived in the turn
void waitForBackSensor(Direction dir) {
	volatile Sensor_data sensor_data;
	double sensor;
	do {
		sensor_data = getSensorData();
		if(dir == LEFT){
			sensor = sensor_data.bl;
		}else{
			sensor = sensor_data.br;
		}
		_delay_ms(20);
		
	} while (sensor < 70);
	
}

void celebrate(void) {
	Bus_data stop;
	stop.id = STOP_TIMER;
	stop.count =  command_lengths[STOP_TIMER];
	send_data(which_device[STOP_TIMER], stop);
	send_move_data(0, 0, 0);//stop
	
	TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
	TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
	autoMode = 0;
	pdFlag = 0;
	makeDecisionFlag = 0;
}

void makeDecision(void) {
	volatile Sensor_data sensor_data = getSensorData();
	
	if(sensor_data.fr>150 && sensor_data.fl>150) {
		celebrate();
	} else if(sensor_data.fl>150) {
		send_move_data(0.5, 0, 0);//go forward
		waitForBackSensor(LEFT);
		sensor_data = getSensorData();
		_delay_ms(25);
		if(sensor_data.fr>150) {
			celebrate();
		} else {
			send_move_data(0.5, 0, -0.8);//turn left
			waitForGyro(70);//Wait for 90 degree turn, by asking gyro
			send_move_data(0.5, 0, 0);//go forward
			waitForCorrectValues();
			stableValues = 2;
			turn = 1;
		}
	} else if(sensor_data.fr>150) {
		send_move_data(0.5, 0, 0);//go forward
		waitForBackSensor(RIGHT);
		sensor_data = getSensorData();
		_delay_ms(25);
		if(sensor_data.fl>150) {
			celebrate();
		} else {
			send_move_data(0.5, 0, 0.8);//turn right
			waitForGyro(70);//Wait for 90 degree turn, by asking gyro
			send_move_data(0.5, 0, 0);//go forward
			waitForCorrectValues();
			stableValues = 2;
			turn = 1;
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
		}else {
			TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
			TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
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
		
		_delay_ms(20);
    }
}
