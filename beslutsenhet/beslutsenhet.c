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

//Set CPU clock
#define F_CPU 14745600UL

uint8_t MAX_ADJUSTMENT = 1; //Constant to stop the robot from turning like crazy
double P = 1/30; //Constant for the proportional part
double D = 1/30; //Constant for the derivative part
double prevError = 0; //The previous error

volatile uint8_t autoMode;
volatile uint8_t makeDecisionFlag;
volatile uint8_t pdFlag;


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

typedef union {
	Bus_data bus_data;
	struct {
		uint8_t count;
		data_id id;
		double fr;
		double br;
		double fl;
		double f;
		double bl;
	};
} Sensor_data;

int pdAlgoritm(double distanceRight, double distanceLeft) {
	double error = distanceRight - distanceLeft;
	double adjustment = P*error + D*(error - prevError);
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

void makeDecision(void) {
	Sensor_data sensor_data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		fetch_data(SENSOR, &master_data_to_receive);
		sensor_data = (Sensor_data)master_data_to_receive;
	}
	uint8_t commands[1];
	if(sensor_data.fr<150) {
		if(sensor_data.fl<150) {
			//commands[0] = FORWARD;
			//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);//go forward
		}else {
			if(sensor_data.f<30) {
				//commands[0] = LEFT;
				//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);// 90 left
				//commands[0] = FORWARD;
				//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);//go forward
			}else {
				if(sensor_data.fr<80) {
					//commands[0] = LEFT;
					//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);// 90 left
					//commands[0] = FORWARD;
					//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);//go forward
				}
			}
		}
	}else {
		if(sensor_data.f<30) {
			//commands[0] = RIGHT;
			//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);// 90 left
			//commands[0] = FORWARD;
			//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);//go forward
		}else {
			if(sensor_data.fl<80) {
				//commands[0] = RIGHT;
				//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);// 90 left
				//commands[0] = FORWARD;
				//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);//go forward
			}else {
				//commands[0] = STOP_TIMER;
				//send_to_bus(COMMUNICATION, COMMAND_DATA, 1, commands);//celebrate
				//commands[0] = STOP;
				//send_to_bus(CONTROL, COMMAND_DATA, 1, commands);//stop
				
				autoMode = 0;
				TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
				TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
			}
		}
	}
}

ISR(TIMER1_OVF_vect) {
	makeDecisionFlag = 1;
	TCNT1H = 0x80; //Reset Timer1 high register
	TCNT1L = 0x00; //Reset Timer1 low register
}

ISR(TIMER3_OVF_vect) {
	pdFlag = 1;
	TCNT3H = 0x80; //Reset Timer3 high register
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
	uint8_t commands[1];
	data_to_receive = data;
	if(data_to_receive.id == SET_P) {
		P = data_to_receive.data[0];
	}else if(data_to_receive.id == SET_D) {
		D = data_to_receive.data[0];
	}else if(data_to_receive.data[0] == CHANGEMODE) {
		if(autoMode == 0) {//Change mode
			autoMode = 1;
			TIMSK1 |= (1<<TOIE1);//Enable timer overflow interrupt for Timer1
			TIMSK3 |= (1<<TOIE3);//Enable timer overflow interrupt for Timer3
			commands[0] = START_TIMER;
			//send_to_bus(COMMUNICATION, COMMAND_DATA, 1, commands);
		}else {
			autoMode = 0;
			TIMSK1 &= ~(1<<TOIE1);//Disable timer overflow interrupt for Timer1
			TIMSK3 &= ~(1<<TOIE3);//Disable timer overflow interrupt for Timer3
			commands[0] = STOP_TIMER;
			//send_to_bus(COMMUNICATION, COMMAND_DATA, 1, commands);
		}
	}
}

int main(void) {
	set_as_slave(prepare_data, interpret_data, DECISION);
	set_as_master(F_CPU);
	
	makeDecisionFlag = 0;
	pdFlag = 0;
	autoMode = 0;
	initTimer();
	sei();
    while(1) {
		
        if(makeDecisionFlag == 1) {
			makeDecision();
			makeDecisionFlag = 0;
		}
		
		if(pdFlag == 1) {
			Sensor_data sensor_data;
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				fetch_data(SENSOR, &master_data_to_receive);
				sensor_data = (Sensor_data)master_data_to_receive;
			}
			double adjustment = pdAlgoritm(sensor_data.br, sensor_data.bl);
			//send_to_bus(CONTROL, PD_DATA, 1, commands);
			pdFlag = 0;
		}
    }
}
