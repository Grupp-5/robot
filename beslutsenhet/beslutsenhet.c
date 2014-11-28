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


double MAX_ADJUSTMENT = 0.5; //Constant to stop the robot from turning like crazy
double P = 1.0/40.0; //Constant for the proportional part
double D = 1.0/5.0; //Constant for the derivative part
double prevError = 0; //The previous error

volatile uint8_t autoMode;
volatile uint8_t makeDecisionFlag;
volatile uint8_t pdFlag;
volatile uint8_t turn;


Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};
Bus_data master_data_to_send = {0};
Bus_data master_data_to_receive = {0};
	
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


void pdAlgoritm(double distanceRight, double distanceLeft) {
	double error = distanceRight - distanceLeft;
	double turn_adjustment = 0;
	double side_adjustment = 0;
	if(turn == 1) {
		turn_adjustment =  D*(error - prevError);
		
		if(turn_adjustment > MAX_ADJUSTMENT)
		{
			turn_adjustment = MAX_ADJUSTMENT;
		}
		else if(turn_adjustment < -MAX_ADJUSTMENT)
		{
			turn_adjustment = -MAX_ADJUSTMENT;
		}
		
		if(fabs(turn_adjustment) < 0.01)
		{
			turn = 0;
		}
	} else {
		side_adjustment = P*error;
		if(side_adjustment > MAX_ADJUSTMENT)
		{
			side_adjustment = MAX_ADJUSTMENT;
		}
		else if(side_adjustment < -MAX_ADJUSTMENT)
		{
			side_adjustment = -MAX_ADJUSTMENT;
		}
		
		if(fabs(side_adjustment) < 0.2)
		{
			turn = 1;
		}
	}
	
	PD_Data pd_data = {
		.id = PD_DATA,
		.count = command_lengths[PD_DATA] + 2,
		.error = error,
		.p = side_adjustment,
		.d = turn_adjustment
	};
	
	send_data(COMMUNICATION, pd_data.bus_data);
	
	prevError = error;
	
	send_move_data(0.5, side_adjustment, turn_adjustment);
}

void makeDecision(void) {
	volatile Sensor_data sensor_data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		master_data_to_receive.count = command_lengths[SENSOR_DATA]+2;
		fetch_data(SENSOR, &master_data_to_receive);
		sensor_data = (Sensor_data)master_data_to_receive;
	}
	
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
				send_move_data(0.4, 0, -0.8);//turn left
				_delay_ms(2200);
				//Wait for 90 degree turn, by asking gyro
				send_move_data(0.5, 0, 0);//go forward
				_delay_ms(2500);
			}else {
				if(sensor_data.fr<80) {
					send_move_data(0.4, 0, -0.8);//turn left
					_delay_ms(2200);
					//Wait for 90 degree turn, by asking gyro
					send_move_data(0.5, 0, 0);//go forward
					_delay_ms(2500);
				}
			}
		}
	}else {
		if(sensor_data.f<30) {
			send_move_data(0.4, 0, 0.8);//turn right
			_delay_ms(2200);
			//Wait for 90 degree turn, by asking gyro
			send_move_data(0.5, 0, 0);//go forward
			_delay_ms(2500);
		}else {
			if(sensor_data.fl<80) {
				send_move_data(0.4, 0, 0.8);//turn right
				_delay_ms(2200);
				//Wait for 90 degree turn, by asking gyro
				send_move_data(0.5, 0, 0);//go forward
				_delay_ms(2500);
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
	TCNT3H = 0xB0; //Reset Timer3 high register
	TCNT3L = 0x00; //Reset Timer3 low register
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
	set_as_slave(prepare_data, interpret_data, DECISION);
	set_as_master(F_CPU);
	
	makeDecisionFlag = 0;
	pdFlag = 0;
	autoMode = 0;
	turn = 1;
	initTimer();
	sei();
    while(1) {
		
        if(makeDecisionFlag == 1) {
			makeDecision();
			makeDecisionFlag = 0;
		}
		
		_delay_ms(20);
		
		if(pdFlag == 1) {
			volatile Sensor_data sensor_data;
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				master_data_to_receive.count = command_lengths[SENSOR_DATA]+2;
				fetch_data(SENSOR, &master_data_to_receive);
				sensor_data = (Sensor_data)master_data_to_receive;
			}
			pdAlgoritm(sensor_data.br, sensor_data.bl);
			pdFlag = 0;
		}
		
		_delay_ms(20);
    }
}
